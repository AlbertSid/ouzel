// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <vector>
#include "math/Quaternion.hpp"
#include "math/Vector3.hpp"

namespace ouzel
{
    class Engine;

    namespace audio
    {
        class AudioDevice;
        class Listener;

        class Audio
        {
            friend Engine;
        public:
            enum class Driver
            {
                DEFAULT,
                EMPTY,
                OPENAL,
                DIRECTSOUND,
                XAUDIO2,
                OPENSL,
                COREAUDIO,
                ALSA
            };

            enum Channel
            {
                FRONT_LEFT = 0,
                FRONT_RIGHT = 1,
                CENTER = 2,
                LFE = 3,
                BACK_LEFT = 4,
                BACK_RIGHT = 5,
                SIDE_LEFT = 6,
                SIDE_RIGHT = 7
            };

            enum ChannelConfiguration
            {
                MONO,
                STEREO,
                QUAD,
                SURROUND51,
                SURROUND51_REAR,
                SURROUND61,
                SURROUND71
            };

            enum class Format
            {
                SINT16,
                FLOAT32
            };

            ~Audio();

            Audio(const Audio&) = delete;
            Audio& operator=(const Audio&) = delete;

            Audio(const Audio&&) = delete;
            Audio& operator=(const Audio&&) = delete;

            static std::set<Audio::Driver> getAvailableAudioDrivers();

            inline AudioDevice* getDevice() const { return device.get(); }

            bool update();

            void executeOnAudioThread(const std::function<void(void)>& func);

            void addListener(Listener* listener);
            void removeListener(Listener* listener);

            static void resampleLerp(const std::vector<float>& src, uint32_t srcFrames,
                                     std::vector<float>& dst, uint32_t dstFrames,
                                     uint32_t channels);

        protected:
            Audio(Driver driver);
            bool init(bool debugAudio);

            std::unique_ptr<AudioDevice> device;

            std::vector<Listener*> listeners;
        };
    } // namespace audio
} // namespace ouzel
