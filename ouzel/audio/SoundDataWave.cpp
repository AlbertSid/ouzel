// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <iterator>
#include "SoundDataWave.hpp"
#include "StreamWave.hpp"
#include "core/Engine.hpp"
#include "files/FileSystem.hpp"
#include "utils/Log.hpp"
#include "utils/Utils.hpp"

static const uint16_t WAVE_FORMAT_PCM = 0x0001;
static const uint16_t WAVE_FORMAT_IEEE_FLOAT = 0x0003;

namespace ouzel
{
    namespace audio
    {
        SoundDataWave::SoundDataWave()
        {
        }

        bool SoundDataWave::init(const std::string& newFilename)
        {
            filename = newFilename;

            std::vector<uint8_t> newData;
            if (!sharedEngine->getFileSystem()->readFile(newFilename, newData))
            {
                return false;
            }

            return init(newData);
        }

        bool SoundDataWave::init(const std::vector<uint8_t>& newData)
        {
            uint32_t offset = 0;

            if (newData.size() < 16) // RIFF + size + WAVE
            {
                Log(Log::Level::ERR) << "Failed to load sound file, file too small";
                return false;
            }

            if (newData[offset + 0] != 'R' ||
                newData[offset + 1] != 'I' ||
                newData[offset + 2] != 'F' ||
                newData[offset + 3] != 'F')
            {
                Log(Log::Level::ERR) << "Failed to load sound file, not a RIFF format";
                return false;
            }

            offset += 4;

            uint32_t length = decodeUInt32Little(newData.data() + offset);

            offset += 4;

            if (newData.size() != length + 8)
            {
                Log(Log::Level::ERR) << "Failed to load sound file, size mismatch";
            }

            if (newData[offset + 0] != 'W' ||
                newData[offset + 1] != 'A' ||
                newData[offset + 2] != 'V' ||
                newData[offset + 3] != 'E')
            {
                Log(Log::Level::ERR) << "Failed to load sound file, not a WAVE file";
                return false;
            }

            offset += 4;

            bool formatChunkFound = false;
            bool dataChunkFound = false;

            uint16_t bitsPerSample = 0;
            uint16_t formatTag = 0;
            std::vector<uint8_t> soundData;

            for (; offset < newData.size();)
            {
                if (newData.size() < offset + 8)
                {
                    Log(Log::Level::ERR) << "Failed to load sound file, not enough data to read chunk";
                    return false;
                }

                uint8_t chunkHeader[4];
                chunkHeader[0] = newData[offset + 0];
                chunkHeader[1] = newData[offset + 1];
                chunkHeader[2] = newData[offset + 2];
                chunkHeader[3] = newData[offset + 3];

                offset += 4;

                uint32_t chunkSize = decodeUInt32Little(newData.data() + offset);
                offset += 4;

                if (newData.size() < offset + chunkSize)
                {
                    Log(Log::Level::ERR) << "Failed to load sound file, not enough data to read chunk";
                    return false;
                }

                if (chunkHeader[0] == 'f' && chunkHeader[1] == 'm' && chunkHeader[2] == 't' && chunkHeader[3] == ' ')
                {
                    if (chunkSize < 16)
                    {
                        Log(Log::Level::ERR) << "Failed to load sound file, not enough data to read chunk";
                        return false;
                    }

                    uint32_t i = offset;

                    formatTag = decodeUInt16Little(newData.data() + i);
                    i += 2;

                    channels = decodeUInt16Little(newData.data() + i);
                    i += 2;

                    sampleRate = decodeUInt32Little(newData.data() + i);
                    i += 4;

                    i += 4; // average bytes per second

                    i += 2; // block align

                    bitsPerSample = decodeUInt16Little(newData.data() + i);
                    i += 2;

                    formatChunkFound = true;
                }
                else if (chunkHeader[0] == 'd' && chunkHeader[1] == 'a' && chunkHeader[2] == 't' && chunkHeader[3] == 'a')
                {
                    soundData.assign(newData.begin() + static_cast<int>(offset), newData.begin() + static_cast<int>(offset + chunkSize));

                    dataChunkFound = true;
                }

                offset += ((chunkSize + 1) & 0xFFFFFFFE);
            }

            if (!formatChunkFound)
            {
                Log(Log::Level::ERR) << "Failed to load sound file, failed to find a format chunk";
                return false;
            }

            if (!dataChunkFound)
            {
                Log(Log::Level::ERR) << "Failed to load sound file, failed to find a data chunk";
                return false;
            }

            if (bitsPerSample != 8 && bitsPerSample != 16 &&
                bitsPerSample != 24 && bitsPerSample != 32)
            {
                Log(Log::Level::ERR) << "Failed to load sound file, unsupported bit depth";
                return false;
            }

            uint32_t bytesPerSample = bitsPerSample / 8;
            uint32_t samples = static_cast<uint32_t>(soundData.size() / bytesPerSample);
            data.resize(samples);

            if (formatTag == WAVE_FORMAT_PCM)
            {
                if (bitsPerSample == 8)
                {
                    for (uint32_t i = 0; i < samples; ++i)
                    {
                        data[i] = 2.0f * static_cast<float>(soundData[i]) / 255.0f - 1.0f;
                    }
                }
                else if (bitsPerSample == 16)
                {
                    for (uint32_t i = 0; i < samples; ++i)
                    {
                        data[i] = static_cast<float>(static_cast<int16_t>(soundData[i * 2] |
                                                                          (soundData[i * 2 + 1] << 8))) / 32767.0f;
                    }
                }
                else if (bitsPerSample == 24)
                {
                    for (uint32_t i = 0; i < samples; ++i)
                    {
                        data[i] = static_cast<float>(static_cast<int32_t>((soundData[i * 3] << 8) |
                                                                          (soundData[i * 3 + 1] << 16) |
                                                                          (soundData[i * 3 + 2] << 24))) / 2147483648.0f;
                    }
                }
                else
                {
                    Log(Log::Level::ERR) << "Failed to load sound file, unsupported bit depth";
                    return false;
                }
            }
            else if (formatTag == WAVE_FORMAT_IEEE_FLOAT)
            {
                if (bitsPerSample == 32)
                {
                    for (uint32_t i = 0; i < samples; ++i)
                    {
                        data[i] = reinterpret_cast<float*>(soundData.data())[i];
                    }
                }
                else
                {
                    Log(Log::Level::ERR) << "Failed to load sound file, unsupported bit depth";
                    return false;
                }
            }
            else
            {
                Log(Log::Level::ERR) << "Failed to load sound file, unsupported format";
                return false;
            }

            return true;
        }

        std::shared_ptr<Stream> SoundDataWave::createStream()
        {
            return std::make_shared<StreamWave>();
        }

        bool SoundDataWave::getData(Stream* stream, uint32_t frames, std::vector<float>& result)
        {
            StreamWave* streamWave = static_cast<StreamWave*>(stream);

            uint32_t offset = streamWave->getOffset();
            uint32_t remainingSize = static_cast<uint32_t>(data.size() - offset);
            uint32_t neededSize = frames * channels;
            uint32_t totalSize = 0;

            result.resize(neededSize);

            while (neededSize > 0)
            {
                if (stream->isRepeating() && remainingSize == 0)
                {
                    streamWave->reset();
                    offset = 0;
                    remainingSize = static_cast<uint32_t>(data.size());
                }

                if (remainingSize < neededSize)
                {
                    std::copy(data.begin() + offset, data.end(), result.begin() + totalSize);
                    streamWave->setOffset(offset + remainingSize);
                    totalSize += remainingSize;
                    neededSize -= remainingSize;
                }
                else
                {
                    std::copy(data.begin() + offset, data.begin() + offset + neededSize, result.begin() + totalSize);
                    streamWave->setOffset(offset + neededSize);
                    totalSize += neededSize;
                    neededSize = 0;
                }

                if (!stream->isRepeating()) break;
            }

            if (remainingSize == 0)
            {
                streamWave->reset();
            }

            result.resize(totalSize);

            return true;
        }
    } // namespace audio
} // namespace ouzel
