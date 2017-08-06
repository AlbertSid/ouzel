// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "core/CompileConfig.hpp"

#if OUZEL_SUPPORTS_XAUDIO2

#include <xaudio2.h>

HRESULT XAudio27CreateProc(IXAudio2** ppXAudio2, UINT32 Flags, UINT32 XAudio2Processor);
ULONG IXAudio2Release(IXAudio2* pXAudio2);
HRESULT IXAudio2CreateMasteringVoice(IXAudio2* pXAudio2,
                                     IXAudio2MasteringVoice** ppMasteringVoice,
                                     UINT32 InputChannels = XAUDIO2_DEFAULT_CHANNELS,
                                     UINT32 InputSampleRate = XAUDIO2_DEFAULT_SAMPLERATE,
                                     UINT32 Flags = 0,
                                     UINT32 DeviceIndex = 0,
                                     const XAUDIO2_EFFECT_CHAIN* pEffectChain = nullptr);
HRESULT IXAudio2CreateSourceVoice(IXAudio2* pXAudio2,
								  IXAudio2SourceVoice** ppSourceVoice,
								  const WAVEFORMATEX* pSourceFormat,
								  UINT32 Flags = 0,
                                  float MaxFrequencyRatio = XAUDIO2_DEFAULT_FREQ_RATIO,
                                  IXAudio2VoiceCallback* pCallback = nullptr,
                                  const XAUDIO2_VOICE_SENDS* pSendList = nullptr,
                                  const XAUDIO2_EFFECT_CHAIN* pEffectChain = nullptr);

#endif
