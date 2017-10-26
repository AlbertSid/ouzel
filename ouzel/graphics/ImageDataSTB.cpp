// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "ImageDataSTB.hpp"
#include "utils/Log.hpp"
#include "core/Engine.hpp"
#include "files/FileSystem.hpp"
#define STBI_NO_PSD
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_GIF
#define STBI_NO_PNM
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace ouzel
{
    namespace graphics
    {
        bool ImageDataSTB::init(const std::string& filename,
                                PixelFormat newPixelFormat)
        {
            std::vector<uint8_t> newData;
            if (!engine->getFileSystem()->readFile(filename, newData))
            {
                return false;
            }

            return init(newData, newPixelFormat);
        }

        bool ImageDataSTB::init(const std::vector<uint8_t>& newData,
                                PixelFormat newPixelFormat)
        {
            int width;
            int height;
            int comp;

            int reqComp;
            switch (newPixelFormat)
            {
                case PixelFormat::R8_UNORM: reqComp = STBI_grey; break;
                case PixelFormat::RG8_UNORM: reqComp = STBI_grey_alpha; break;
                case PixelFormat::RGBA8_UNORM: reqComp = STBI_rgb_alpha; break;
                default: reqComp = STBI_default;
            }

            stbi_uc* tempData = stbi_load_from_memory(newData.data(), static_cast<int>(newData.size()), &width, &height, &comp, reqComp);

            if (!tempData)
            {
                Log(Log::Level::ERR) << "Failed to load texture, reason: " << stbi_failure_reason();
                return false;
            }

            if (reqComp != STBI_default) comp = reqComp;

            size_t pixelSize;
            switch (comp)
            {
                case STBI_grey: pixelFormat = PixelFormat::R8_UNORM; pixelSize = 1; break;
                case STBI_grey_alpha: pixelFormat = PixelFormat::RG8_UNORM; pixelSize = 2; break;
                case STBI_rgb_alpha: pixelFormat = PixelFormat::RGBA8_UNORM; pixelSize = 4; break;
                default:
                    Log(Log::Level::ERR) << "Unknown pixel size";
                    return false;
            }

            data.assign(tempData, tempData + (width * height * pixelSize));

            stbi_image_free(tempData);

            size.width = static_cast<float>(width);
            size.height = static_cast<float>(height);

            return true;
        }

        bool ImageDataSTB::writeToFile(const std::string& newFilename)
        {
            int depth = static_cast<int>(getPixelSize(pixelFormat));
            int width = static_cast<int>(size.width);
            int height = static_cast<int>(size.height);

            if (!stbi_write_png(newFilename.c_str(), width, height, depth, data.data(), width * depth))
            {
                Log(Log::Level::ERR) << "Failed to save image to file";
                return false;
            }

            return true;
        }

    } // namespace graphics
} // namespace ouzel
