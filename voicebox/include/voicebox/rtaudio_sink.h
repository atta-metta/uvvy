//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "audio_sink.h"

namespace voicebox {

/**
 * RtAudio sink uses audio_hardware to playback received data.
 */
class rtaudio_sink : public audio_sink
{
    typedef audio_sink super;

public:
    rtaudio_sink() = default;

    void set_enabled(bool enable) override;
};

} // voicebox namespace
