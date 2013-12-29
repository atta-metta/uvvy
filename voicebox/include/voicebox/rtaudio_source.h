//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "audio_source.h"

namespace voicebox {

/**
 * RtAudio source uses audio_hardware to receive captured input stream.
 *
 * @todo Set a fixed-size ringbuffer to store audio data before packetizer
 * gets it chunked.
 */
class rtaudio_source : public audio_source
{
    typedef audio_source super;

public:
    rtaudio_source() = default;

    void set_enabled(bool enable) override;
};

} // voicebox namespace