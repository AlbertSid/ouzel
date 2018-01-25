// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "core/Setup.h"

#if OUZEL_COMPILE_DIRECT3D11

#include <d3d11.h>
#include "graphics/BlendStateResource.hpp"

namespace ouzel
{
    namespace graphics
    {
        class RenderDeviceD3D11;

        class BlendStateResourceD3D11: public BlendStateResource
        {
        public:
            BlendStateResourceD3D11(RenderDeviceD3D11* initRenderDeviceD3D11);
            virtual ~BlendStateResourceD3D11();

            virtual bool init(bool newEnableBlending,
                              BlendState::Factor newColorBlendSource, BlendState::Factor newColorBlendDest,
                              BlendState::Operation newColorOperation,
                              BlendState::Factor newAlphaBlendSource, BlendState::Factor newAlphaBlendDest,
                              BlendState::Operation newAlphaOperation,
                              uint8_t newColorMask) override;

            ID3D11BlendState* getBlendState() const { return blendState; }

        protected:
            RenderDeviceD3D11* renderDeviceD3D11;

            ID3D11BlendState* blendState = nullptr;
        };
    } // namespace graphics
} // namespace ouzel

#endif
