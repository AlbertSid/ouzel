// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "core/Setup.h"

#if OUZEL_COMPILE_ALSA

#include <atomic>
#include <thread>
#include <alsa/asoundlib.h>

#include "audio/AudioDevice.hpp"

namespace ouzel
{
    namespace audio
    {
        class AudioDeviceALSA: public AudioDevice
        {
            friend Audio;
        public:
            virtual ~AudioDeviceALSA();

        protected:
            AudioDeviceALSA();
            virtual bool init(bool debugAudio) override;

            void run();

            snd_pcm_t* playbackHandle = nullptr;
            snd_pcm_hw_params_t* hwParams = nullptr;
            snd_pcm_sw_params_t* swParams = nullptr;

            unsigned int periods = 4;
            snd_pcm_uframes_t periodSize = 1024;

            std::vector<uint8_t> data;

            std::atomic<bool> running;
            std::thread audioThread;
        };
    } // namespace audio
} // namespace ouzel

#endif
