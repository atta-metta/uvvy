//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2014, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "logging.h"
#include "voicebox/packet_source.h"
#include "voicebox/audio_service.h"

using namespace std;
using namespace ssu;
namespace pt = boost::posix_time;

namespace voicebox {

static const pt::ptime epoch{boost::gregorian::date(2010, boost::gregorian::Jan, 1)};

packet_source::packet_source(std::shared_ptr<ssu::stream> stream)
    : audio_source()
{
    set_source(stream);
}

packet_source::~packet_source()
{
    if (stream_) {
        stream_->shutdown(stream::shutdown_mode::read); // @todo Maybe do this in set_enabled(false)?
    }
}

void packet_source::set_source(std::shared_ptr<ssu::stream> stream)
{
    stream_ = stream;
    stream_->on_ready_read_datagram.connect([this]{ on_packet_received(); });
}

void packet_source::set_enabled(bool enabling)
{
    logger::debug(TRACE_ENTRY) << __PRETTY_FUNCTION__ << " " << enabling;
    super::set_enabled(enabling);
}

/* Put received packet into receive queue */

// NB:
// Should this class handle the received packets or should it already receive the appearing packets
// in its accept_input() method and so essentially do nothing?
// Probably on_packet_received() should be here, but should also check is_enabled() before calling
// accept_input(), so disabling packet_source would just ignore ingress of packets.

void packet_source::on_packet_received()
{
    if (is_enabled()) {
        byte_array msg = stream_->read_datagram();

#if REALTIME_CRIME
        logger::debug(TRACE_ENTRY) << "Received packet of size " << msg.size();
#endif

    // See JB: it sets time_difference_ based on first received remove packet timestamp
    // if (!time_difference_) {
    //     time_difference_ = (pt::microsec_clock::universal_time() - epoch).total_milliseconds()
    //         - msg.as<big_int64_t>()[0];
    // }
        accept_input(msg);
    }
}

// void packet_source::log_packet_delay(byte_array const& pkt)
// {
// #if DELAY_PLOT
//     int64_t ts = pkt.as<big_int64_t>()[0];
//     int64_t local_ts = (pt::microsec_clock::universal_time() - epoch).total_milliseconds();
//     // logger::info() << "Packet ts " << ts << ", local ts " << local_ts << ", play difference "
//         // << fabs(local_ts - ts);

//     plot_.dump(ts, local_ts);
// #endif
// }

} // voicebox namespace
