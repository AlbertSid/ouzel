// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "audio/SoundInput.hpp"
#include "audio/SoundOutput.hpp"

namespace ouzel
{
    namespace audio
    {
        class Mixer: public SoundInput, public SoundOutput
        {
        public:
            Mixer();
            virtual ~Mixer();

            Mixer(const Mixer&) = delete;
            Mixer& operator=(const Mixer&) = delete;

            Mixer(const Mixer&&) = delete;
            Mixer& operator=(const Mixer&&) = delete;

            inline float getPitch() const { return pitch; }
            inline void setPitch(float newPitch) { pitch = newPitch; }

            inline float getGain() const { return gain; }
            inline void setGain(float newGain) { gain = newGain; }

            inline float getRolloffScale() const { return rolloffScale; }
            inline void setRolloffScale(float newRolloffScale) { rolloffScale = newRolloffScale; }

            virtual void addRenderCommands(std::vector<AudioDevice::RenderCommand>& renderCommands) override;

        protected:
            static void setAttributes(Vector3& listenerPosition,
                                      Quaternion& listenerRotation,
                                      float& pitch,
                                      float& gain,
                                      float& rolloffFactor,
                                      float pitchScale,
                                      float gainScale,
                                      float rolloffScale);

            float pitch = 1.0f;
            float gain = 1.0f;
            float rolloffScale = 1.0f;
        };
    } // namespace audio
} // namespace ouzel
