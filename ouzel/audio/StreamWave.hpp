// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <cstdint>
#include "Stream.hpp"

namespace ouzel
{
    namespace audio
    {
        class StreamWave: public Stream
        {
        public:
            virtual void reset() override;

            inline uint32_t getOffset() const { return offset; }
            inline void setOffset(uint32_t newOffset) { offset = newOffset; }

        private:
            uint32_t offset = 0;
        };
    } // namespace audio
} // namespace ouzel
