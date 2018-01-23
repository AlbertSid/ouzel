// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "audio/AudioDevice.hpp"

namespace ouzel
{
    namespace audio
    {
        class SoundOutput;

        class SoundInput
        {
            friend SoundOutput;
        public:
            virtual ~SoundInput();

            SoundOutput* getOutput() const { return output; }
            void setOutput(SoundOutput* newOutput);

            virtual void addRenderCommands(std::vector<AudioDevice::RenderCommand>& renderCommands) = 0;

        protected:
            SoundOutput* output = nullptr;
        };
    } // namespace audio
} // namespace ouzel
