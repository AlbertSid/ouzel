// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

namespace ouzel
{
    namespace graphics
    {
        enum class PixelFormat
        {
            DEFAULT,
            R8_UNORM,
            R8_SNORM,
            R8_UINT,
            R8_SINT,
            R16_UNORM,
            R16_SNORM,
            R16_UINT,
            R16_SINT,
            R16_FLOAT,
            R32_UINT,
            R32_SINT,
            R32_FLOAT,
            RG8_UNORM,
            RG8_SNORM,
            RG8_UINT,
            RG8_SINT,
            RGB8_UNORM,
            RGB8_SNORM,
            RGB8_UINT,
            RGB8_SINT,
            RGBA8_UNORM,
            RGBA8_SNORM,
            RGBA8_UINT,
            RGBA8_SINT,
            ABGR8_UNORM,
            RGBA16_UNORM,
            RGBA16_SNORM,
            RGBA16_UINT,
            RGBA16_SINT,
            RGBA16_FLOAT,
            RGBA32_UINT,
            RGBA32_SINT,
            RGBA32_FLOAT,
            R5G5B5A1_UNORM
        };

        inline uint32_t getPixelSize(PixelFormat pixelFormat)
        {
            switch (pixelFormat)
            {
                case PixelFormat::R8_UNORM:
                case PixelFormat::R8_SNORM:
                case PixelFormat::R8_UINT:
                case PixelFormat::R8_SINT:
                    return 1;
                case PixelFormat::R16_UNORM:
                case PixelFormat::R16_SNORM:
                case PixelFormat::R16_UINT:
                case PixelFormat::R16_SINT:
                case PixelFormat::R16_FLOAT:
                    return 2;
                case PixelFormat::R32_UINT:
                case PixelFormat::R32_SINT:
                case PixelFormat::R32_FLOAT:
                    return 4;
                case PixelFormat::RG8_UNORM:
                case PixelFormat::RG8_SNORM:
                case PixelFormat::RG8_UINT:
                case PixelFormat::RG8_SINT:
                    return 2;
                case PixelFormat::RGB8_UNORM:
                case PixelFormat::RGB8_SNORM:
                case PixelFormat::RGB8_UINT:
                case PixelFormat::RGB8_SINT:
                    return 3;
                case PixelFormat::RGBA8_UNORM:
                case PixelFormat::RGBA8_SNORM:
                case PixelFormat::RGBA8_UINT:
                case PixelFormat::RGBA8_SINT:
                case PixelFormat::ABGR8_UNORM:
                    return 4;
                case PixelFormat::RGBA16_UNORM:
                case PixelFormat::RGBA16_SNORM:
                case PixelFormat::RGBA16_UINT:
                case PixelFormat::RGBA16_SINT:
                case PixelFormat::RGBA16_FLOAT:
                    return 8;
                case PixelFormat::RGBA32_UINT:
                case PixelFormat::RGBA32_SINT:
                case PixelFormat::RGBA32_FLOAT:
                    return 16;
                case PixelFormat::R5G5B5A1_UNORM:
                    return 2;
                default:
                    return 0;
            }
        }
    } // namespace graphics
} // namespace ouzel
