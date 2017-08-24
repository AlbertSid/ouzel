// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "core/CompileConfig.h"

#if OUZEL_SUPPORTS_XAUDIO2

#include "AudioDeviceXA2.hpp"
#include "XAudio27.hpp"
#include "audio/SoundResource.hpp"
#include "utils/Log.hpp"

static const char* XAUDIO2_DLL_28 = "xaudio2_8.dll";
static const char* XAUDIO2_DLL_27 = "xaudio2_7.dll";

typedef HRESULT(__stdcall *XAudio2CreateProc)(IXAudio2** ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor);

namespace ouzel
{
    namespace audio
    {
        AudioDeviceXA2::AudioDeviceXA2():
            AudioDevice(Audio::Driver::XAUDIO2)
        {
        }

        AudioDeviceXA2::~AudioDeviceXA2()
        {
            if (sourceVoice) sourceVoice->DestroyVoice();
            if (masteringVoice) masteringVoice->DestroyVoice();
            if (xAudio)
            {
                if (apiMajorVersion == 2 && apiMinorVersion == 7) IXAudio2Release(xAudio);
                else xAudio->Release();
            }
            if (xAudio2Library) FreeModule(xAudio2Library);
        }

        bool AudioDeviceXA2::init(bool debugAudio)
        {
            if (!AudioDevice::init(debugAudio))
            {
                return false;
            }

            xAudio2Library = LoadLibraryA(XAUDIO2_DLL_28);

            if (xAudio2Library)
            {
                apiMajorVersion = 2;
                apiMinorVersion = 8;

                XAudio2CreateProc xAudio2CreateProc = reinterpret_cast<XAudio2CreateProc>(GetProcAddress(xAudio2Library, "XAudio2Create"));

                if (!xAudio2CreateProc)
                {
                    Log(Log::Level::ERR) << "Failed to get address of XAudio2Create";
                    return false;
                }

                HRESULT hr = xAudio2CreateProc(&xAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
                if (FAILED(hr))
                {
                    Log(Log::Level::ERR) << "Failed to initialize XAudio2, error: " << hr;
                    return false;
                }

                if (debugAudio)
                {
                    XAUDIO2_DEBUG_CONFIGURATION debugConfiguration;
                    debugConfiguration.TraceMask = XAUDIO2_LOG_WARNINGS | XAUDIO2_LOG_DETAIL;
                    debugConfiguration.BreakMask = XAUDIO2_LOG_ERRORS;
                    debugConfiguration.LogThreadID = FALSE;
                    debugConfiguration.LogFileline = FALSE;
                    debugConfiguration.LogFunctionName = FALSE;
                    debugConfiguration.LogTiming = FALSE;

                    xAudio->SetDebugConfiguration(&debugConfiguration);
                }
            }
            else
            {
                Log(Log::Level::INFO) << "Failed to load " << XAUDIO2_DLL_28;

                xAudio2Library = LoadLibraryA(XAUDIO2_DLL_27);

                if (xAudio2Library)
                {
                    apiMajorVersion = 2;
                    apiMinorVersion = 7;
                }
                else
                {
                    Log(Log::Level::ERR) << "Failed to load " << XAUDIO2_DLL_27;
                    return false;
                }

                const UINT XAUDIO2_DEBUG_ENGINE = 0x0001;

                UINT32 flags = 0;
                if (debugAudio) flags |= XAUDIO2_DEBUG_ENGINE;

                HRESULT hr = XAudio27CreateProc(&xAudio, flags, XAUDIO2_DEFAULT_PROCESSOR);
                if (FAILED(hr))
                {
                    Log(Log::Level::ERR) << "Failed to initialize XAudio2, error: " << hr;
                    return false;
                }
            }

            WAVEFORMATEX waveFormat;
            waveFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
            waveFormat.nChannels = channels;
            waveFormat.nSamplesPerSec = sampleRate;
            waveFormat.wBitsPerSample = 32;
            waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);
            waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
            waveFormat.cbSize = 0;

            if (apiMajorVersion == 2 && apiMinorVersion == 7)
            {
                HRESULT hr = IXAudio2CreateMasteringVoice(xAudio, &masteringVoice);
                if (FAILED(hr))
                {
                    Log(Log::Level::ERR) << "Failed to create XAudio2 mastering voice, error: " << hr;
                    return false;
                }

                hr = IXAudio2CreateSourceVoice(xAudio, &sourceVoice, &waveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this);
                if (FAILED(hr))
                {
                    Log(Log::Level::ERR) << "Failed to create source voice, error: " << hr;
                    return false;
                }
            }
            else
            {
                HRESULT hr = xAudio->CreateMasteringVoice(&masteringVoice);
                if (FAILED(hr))
                {
                    Log(Log::Level::ERR) << "Failed to create XAudio2 mastering voice, error: " << hr;
                    return false;
                }

                hr = xAudio->CreateSourceVoice(&sourceVoice, &waveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this);
                if (FAILED(hr))
                {
                    Log(Log::Level::ERR) << "Failed to create source voice, error: " << hr;
                    return false;
                }
            }

            getData(bufferSize / sizeof(int16_t), Audio::Format::SINT16, data[0]);

            XAUDIO2_BUFFER bufferData;
            bufferData.Flags = 0;
            bufferData.AudioBytes = static_cast<UINT32>(data[0].size());
            bufferData.pAudioData = data[0].data();
            bufferData.PlayBegin = 0;
            bufferData.PlayLength = 0;
            bufferData.LoopBegin = 0;
            bufferData.LoopLength = 0;
            bufferData.LoopCount = 0;
            bufferData.pContext = nullptr;

            HRESULT hr = sourceVoice->SubmitSourceBuffer(&bufferData);
            if (FAILED(hr))
            {
                Log(Log::Level::ERR) << "Failed to upload sound data, error: " << hr;
                return false;
            }

            getData(bufferSize / sizeof(int16_t), Audio::Format::SINT16, data[1]);
            bufferData.AudioBytes = static_cast<UINT32>(data[1].size());
            bufferData.pAudioData = data[1].data();

            hr = sourceVoice->SubmitSourceBuffer(&bufferData);
            if (FAILED(hr))
            {
                Log(Log::Level::ERR) << "Failed to upload sound data, error: " << hr;
                return false;
            }

            nextBuffer = 0;

            hr = sourceVoice->Start();
            if (FAILED(hr))
            {
                Log(Log::Level::ERR) << "Failed to start consuming sound data, error: " << hr;
                return false;
            }

            return true;
        }

        void AudioDeviceXA2::OnVoiceProcessingPassStart(UINT32)
        {
        }
        void AudioDeviceXA2::OnVoiceProcessingPassEnd()
        {
        }
        void AudioDeviceXA2::OnStreamEnd()
        {
        }
        void AudioDeviceXA2::OnBufferStart(void*)
        {
        }
        void AudioDeviceXA2::OnBufferEnd(void*)
        {
            if (!update())
            {
                return;
            }

            if (!getData(bufferSize / sizeof(float), Audio::Format::FLOAT32, data[nextBuffer]))
            {
                return;
            }

            XAUDIO2_BUFFER bufferData;
            bufferData.Flags = 0;
            bufferData.AudioBytes = static_cast<UINT32>(data[nextBuffer].size());
            bufferData.pAudioData = data[nextBuffer].data();
            bufferData.PlayBegin = 0;
            bufferData.PlayLength = 0;
            bufferData.LoopBegin = 0;
            bufferData.LoopLength = 0;
            bufferData.LoopCount = 0;
            bufferData.pContext = nullptr;

            HRESULT hr = sourceVoice->SubmitSourceBuffer(&bufferData);
            if (FAILED(hr))
            {
                Log(Log::Level::ERR) << "Failed to upload sound data, error: " << hr;
            }

            nextBuffer = (nextBuffer == 0) ? 1 : 0;
        }
        void AudioDeviceXA2::OnLoopEnd(void*)
        {
        }
        void AudioDeviceXA2::OnVoiceError(void*, HRESULT error)
        {
            Log() << "Xaudio2 voice error: " << error;
        }

    } // namespace audio
} // namespace ouzel

#endif
