//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/positional_options.hpp>
#include <queue>
#include <mutex>
#include "host.h"
#include "stream.h"
#include "server.h"
#include "logging.h"
#include "opus.h"
#include "RtAudio.h"
#include "algorithm.h"
#include "settings_provider.h"

#include "private/regserver_client.h" // @fixme Testing only.

using namespace std;
using namespace ssu;

// Receive packets from remote
// Opus-decode and playback
class audio_receiver
{
    OpusDecoder *decode_state_{0};
    size_t frame_size_{0};
    int rate_{0};
    std::mutex queue_mutex_;
    std::queue<byte_array> packet_queue_;
    shared_ptr<stream> stream_;
    boost::asio::strand strand_;

public:
    static constexpr int nChannels{1};

    audio_receiver(shared_ptr<host> host)
        : strand_(host->get_io_service())
    {
        int error{0};
        decode_state_ = opus_decoder_create(48000, nChannels, &error);
        assert(decode_state_);
        assert(!error);

        opus_decoder_ctl(decode_state_, OPUS_GET_SAMPLE_RATE(&rate_));
        frame_size_ = rate_ / 100; // 10ms
    }

    ~audio_receiver()
    {
        if (stream_) {
            stream_->shutdown(stream::shutdown_mode::read);
        }
        opus_decoder_destroy(decode_state_); decode_state_ = 0;
    }

    void streaming(shared_ptr<stream> stream)
    {
        stream_ = stream;
        stream_->on_ready_read_datagram.connect([this]{ on_packet_received(); });
    }

    /**
     * Decode a packet into the provided buffer; decode a missing frame if queue is empty.
     * @param[out] decoded_packet Buffer for decoded packet.
     * @param[in] max_frames Maximum number of floating point frames in provided buffer.
     */
    void get_packet(float* decoded_packet, size_t max_frames)
    {
        logger::debug() << "get_packet";
        std::unique_lock<std::mutex> lock(queue_mutex_);
        assert(max_frames == frame_size_);

        if (packet_queue_.size() > 0)
        {
            byte_array pkt = packet_queue_.front();
            packet_queue_.pop();
            lock.unlock();
            int len = opus_decode_float(decode_state_, (unsigned char*)pkt.data(), pkt.size(), decoded_packet, frame_size_, /*decodeFEC:*/0);
            assert(len > 0);
            assert(len == int(frame_size_));
            logger::debug() << "get_packet decoded frame of size " << pkt.size() << " into " << len << " frames";
        } else {
            lock.unlock();
            // "decode" a missing frame
            int len = opus_decode_float(decode_state_, NULL, 0, decoded_packet, frame_size_, /*decodeFEC:*/0);
            assert(len > 0);
            logger::debug() << "get_packet decoded missing frame of size " << len;
            // assert(len == frame_size_);
        }
    }

protected:
    /* Put received packet into receive queue */
    void on_packet_received()
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        // extract payload
        byte_array msg = stream_->read_datagram();
        logger::debug() << "received packet of size " << msg.size();
        packet_queue_.push(msg);
    }
};

// Capture and opus-encode audio
// send it to remote endpoint
class audio_sender
{
    OpusEncoder *encode_state_{0};
    int frame_size_{0}, rate_{0};
    shared_ptr<stream> stream_;
    boost::asio::strand strand_;

public:
    static constexpr int nChannels{1};

    audio_sender(shared_ptr<host> host)
        : strand_(host->get_io_service())
    {
        int error{0};
        encode_state_ = opus_encoder_create(48000, nChannels, OPUS_APPLICATION_VOIP, &error);
        assert(encode_state_);
        assert(!error);

        opus_encoder_ctl(encode_state_, OPUS_GET_SAMPLE_RATE(&rate_));
        frame_size_ = rate_ / 100; // 10ms

        opus_encoder_ctl(encode_state_, OPUS_SET_BITRATE(OPUS_AUTO));
        opus_encoder_ctl(encode_state_, OPUS_SET_VBR(1));
        opus_encoder_ctl(encode_state_, OPUS_SET_DTX(1));
    }

    ~audio_sender()
    {
        if (stream_) {
            stream_->shutdown(stream::shutdown_mode::write);
        }
        opus_encoder_destroy(encode_state_); encode_state_ = 0;
    }

    /**
     * Use given stream for sending out audio packets.
     * @param stream Stream handed in from server->accept().
     */
    void streaming(shared_ptr<stream> stream)
    {
        stream_ = stream;
    }

    // Called by rtaudio callback to encode and send packet.
    void send_packet(float* buffer, size_t nFrames)
    {
        logger::debug() << "send_packet frame size " << frame_size_ << ", got nFrames " << nFrames;
        assert((int)nFrames == frame_size_);
        byte_array samplebuf(nFrames*sizeof(float));
        opus_int32 nbytes = opus_encode_float(encode_state_, buffer, nFrames, (unsigned char*)samplebuf.data(), nFrames*sizeof(float));
        assert(nbytes > 0);
        samplebuf.resize(nbytes);
        strand_.post([this, samplebuf]{
            stream_->write_datagram(samplebuf, stream::datagram_type::non_reliable);
        });        
    }
};

class audio_hardware
{
    RtAudio* audio_inst{0};
    audio_sender* sender_{0};
    audio_receiver* receiver_{0};

public:
    audio_hardware(audio_sender* sender, audio_receiver* receiver)
        : sender_(sender)
        , receiver_(receiver)
    {
        try {
            audio_inst  = new RtAudio();
        }
        catch (RtError &error) {
            logger::warning() << "Can't initialize RtAudio library, " << error.what();
            return;
        }

        // Open the audio device
        RtAudio::StreamParameters inparam, outparam;
        inparam.deviceId = audio_inst->getDefaultInputDevice();
        inparam.nChannels = audio_sender::nChannels;
        outparam.deviceId = audio_inst->getDefaultOutputDevice();
        outparam.nChannels = audio_receiver::nChannels;
        unsigned int bufferFrames = 480;

        try {
            audio_inst->openStream(&outparam, &inparam, RTAUDIO_FLOAT32, 48000, &bufferFrames, rtcallback, this);
        }
        catch (RtError &error) {
            logger::warning() << "Couldn't open stream, " << error.what();
            return;
        }
    }

    ~audio_hardware()
    {
        try {
            audio_inst->closeStream();
        }
        catch (RtError &error) {
            logger::warning() << "Couldn't close stream, " << error.what();
        }
        delete audio_inst; audio_inst = 0;
    }

    void new_connection(shared_ptr<server> server)
    {
        auto stream = server->accept();
        if (!stream)
            return;

        streaming(stream);

        audio_inst->startStream();
    }

    void out_stream_ready()
    {
        audio_inst->startStream();
    }

    void streaming(shared_ptr<stream> stream)
    {
        sender_->streaming(stream);
        receiver_->streaming(stream);
    }

private:
    static int rtcallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double, RtAudioStreamStatus, void *userdata)
    {
        audio_hardware* instance = reinterpret_cast<audio_hardware*>(userdata);

        logger::debug() << "rtcallback["<<instance<<"] outputBuffer " << outputBuffer << ", inputBuffer " << inputBuffer << ", nframes " << nFrames;

        // An RtAudio "frame" is one sample per channel,
        // whereas our "frame" is one buffer worth of data (as in Speex).

#if 0
        if (inputBuffer && outputBuffer) {
            std::copy_n((char*)inputBuffer, nFrames*sizeof(float), (char*)outputBuffer);
        }
#else
        if (inputBuffer) {
            instance->sender_->send_packet((float*)inputBuffer, nFrames);
        }
        if (outputBuffer) {
            instance->receiver_->get_packet((float*)outputBuffer, nFrames);
        }
#endif
        return 0;
    }
};

namespace po = boost::program_options;

/**
 * Get the address to talk to over IPv6 from the command line.
 * Parses [ipv6::]:port or [ipv6] with default port 9660.
 */
int main(int argc, char* argv[])
{
    bool connect_out{false};
    std::string peer;
    int port = 9660;

    logger::set_verbosity(logger::verbosity::info);

    po::options_description desc("Program arguments");
    desc.add_options()
        ("peer,a", po::value<std::string>(), "Peer EID as base32 identifier")
        ("port,p", po::value<int>(&port), "Run service on this port, connect peer on this port")
        ("help", "Print this help message");
    po::positional_options_description p;
    p.add("peer", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
          options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    settings_provider::set_organization_name("Exquance");
    settings_provider::set_organization_domain("com.exquance");
    settings_provider::set_application_name("opus-streaming");
    auto settings = settings_provider::instance();

    auto s_port = settings->get("port");
    if (!s_port.empty()) {
        port = boost::any_cast<long long>(s_port);
    }

    if (vm.count("port"))
    {
        port = vm["port"].as<int>();
    }

    if (vm.count("peer"))
    {
        peer = vm["peer"].as<std::string>();
        connect_out = true;
    }

    settings->set("port", port);
    settings->sync();

    shared_ptr<host> host(host::create(settings.get(), port));
    shared_ptr<stream> stream;
    shared_ptr<server> server;

    uia::routing::internal::regserver_client regclient(host.get());

    // @todo Pull client profile from settings.
    uia::routing::client_profile client;
    client.set_host_name("aramaki.local");
    client.set_owner_name("Berkus");
    client.set_city("Tallinn");
    client.set_region("Harju");
    client.set_country("Estonia");
    client.set_endpoints(set_to_vector(host->active_local_endpoints()));
    for (auto kw : client.keywords()) {
        logger::debug() << "Keyword: " << kw;
    }
    regclient.set_profile(client);

    // vector<string> regservers = settings->get("regservers");
    regclient.register_at("192.168.1.67");

    audio_receiver receiver(host);
    audio_sender sender(host);
    audio_hardware hw(&sender, &receiver);

    if (connect_out)
    {
        peer_id eid{peer};
        logger::info() << "Connecting to " << eid;

        stream = make_shared<ssu::stream>(host);
        hw.streaming(stream);
        stream->on_link_up.connect([&] { hw.out_stream_ready(); });
        stream->connect_to(eid, "streaming", "opus");
    }
    else
    {
        logger::info() << "Listening on port " << dec << port;

        server = make_shared<ssu::server>(host);
        server->on_new_connection.connect([&] { hw.new_connection(server); });
        bool listening = server->listen("streaming", "Streaming services", "opus", "OPUS Audio protocol");
        assert(listening);
    }

    host->run_io_service();
}
