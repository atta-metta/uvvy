//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/format.hpp>
#include "arsenal/flurry.h"
#include "arsenal/byte_array_wrap.h"
#include "opus.h"

using namespace std;
namespace po = boost::program_options;

/**
 * Record information into a gnuplot-compatible file.
 */
class plotfile
{
    std::mutex m;
    std::ofstream out_;
public:
    plotfile(std::string const& plotname)
        : out_(plotname, std::ios::out|std::ios::trunc|std::ios::binary)
    {
        out_ << "# gnuplot data for packet dump tracing\r\n"
             << "# seq\tpacket_size\tdecoded size\r\n";
    }

    ~plotfile()
    {
        out_ << "\r\n\r\n"; // gnuplot end marker
        out_.close();
    }

    void dump(int64_t seq, int64_t size, int64_t out_size)
    {
        std::unique_lock<std::mutex> lock(m);
        out_ << seq << '\t' << size << '\t' << out_size << "\r\n";
    }
};

struct Decoder
{
    OpusDecoder *decode_state_{0};
    size_t frame_size_{0};
    int rate_{0};

    Decoder()
    {
        int error{0};
        decode_state_ = opus_decoder_create(48000, /*nChannels:*/1, &error);
        assert(decode_state_);
        assert(!error);

        opus_decoder_ctl(decode_state_, OPUS_GET_SAMPLE_RATE(&rate_));
        frame_size_ = rate_ / 100; // 10ms
    }

    ~Decoder()
    {
        opus_decoder_destroy(decode_state_);
        decode_state_ = 0;
    }

    byte_array decode(byte_array const& pkt)
    {
        byte_array decoded_packet(frame_size_*sizeof(float));
        int len = opus_decode_float(decode_state_, (unsigned char*)pkt.data()+8, pkt.size()-8,
            (float*)decoded_packet.data(), frame_size_, /*decodeFEC:*/0);
        assert(len > 0);
        assert(len == int(frame_size_));
        if (len != int(frame_size_)) {
            BOOST_LOG_TRIVIAL(warning) << "Short decode, decoded " << len << " frames, required " << frame_size_;
        }
        return decoded_packet;
    }
};

int main(int argc, char** argv)
{
    std::string filename;

    po::options_description desc("Log file analyzer");
    desc.add_options()
        ("filename,f", po::value<std::string>(&filename)->default_value("dump.bin"),
            "Name of the log dump file")
        ("help,h",
            "Print this help message");
    po::positional_options_description p;
    p.add("filename", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
          options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    byte_array data;
    std::ifstream in(filename, std::ios::in|std::ios::binary);
    flurry::iarchive ia(in);

    plotfile plot_local("local_packetsizes.plot");
    plotfile plot_remote("remote_packetsizes.plot");
    uint32_t old_seq_local = 0;
    uint32_t old_seq_remote = 0;
    Decoder dec_local, dec_remote;
    std::ofstream os_local("local_voice.f32", std::ios::out|std::ios::trunc|std::ios::binary);
    std::ofstream os_remote("remote_voice.f32", std::ios::out|std::ios::trunc|std::ios::binary);

    uint32_t* old_seq = &old_seq_local;
    Decoder* dec = &dec_local;
    std::ofstream* os = &os_local;
    plotfile* plot = &plot_local;

    while (ia >> data) {
        std::string what, stamp;
        byte_array blob;
        byte_array_iwrap<flurry::iarchive> read(data);
        read.archive() >> what >> stamp >> blob;
        // cout << "*** BLOB " << blob.size() << " bytes *** " << stamp << ": " << what << endl;
        data = blob;

        // uint32_t magic = data.as<big_uint32_t>()[0];
        // if ((magic & 0xff000000) == 0) {
        //     continue; // Ignore control packets
        // }
        // uint32_t seq = magic & 0xffffff;
        // if (*old_seq != seq) {
        //     if (seq - *old_seq > 1) {
        //         BOOST_LOG_TRIVIAL(warning) << "Non-consecutive sequence numbers " << *old_seq << "->" << seq;
        //     }
        //     *old_seq = seq;
        //     //continue; -- in the old style decoder
        // }

        if (what == "encoded opus packet") {
            old_seq = &old_seq_local;
            dec = &dec_local;
            os = &os_local;
            plot = &plot_local;
        }
        else if (what == "opus packet before decode") {
            old_seq = &old_seq_remote;
            dec = &dec_remote;
            os = &os_remote;
            plot = &plot_remote;
        }
        else {
            continue;
        }

        if (data.size() < 8) {
            BOOST_LOG_TRIVIAL(warning) << "Packet too small to decode";
            continue;
        }
        //get time stamp from stamp and plot it too, per packet
        int64_t ts = data.as<big_int64_t>()[0];

        byte_array out = dec->decode(data);
        plot->dump(ts, data.size(), out.size());
        os->write(out.data(), out.size());
    }
}
