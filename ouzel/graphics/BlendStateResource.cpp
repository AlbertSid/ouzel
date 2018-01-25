// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "BlendStateResource.hpp"

namespace ouzel
{
    namespace graphics
    {
        BlendStateResource::BlendStateResource()
        {
        }

        BlendStateResource::~BlendStateResource()
        {
        }

        bool BlendStateResource::init(bool newEnableBlending,
                                      BlendState::Factor newColorBlendSource, BlendState::Factor newColorBlendDest,
                                      BlendState::Operation newColorOperation,
                                      BlendState::Factor newAlphaBlendSource, BlendState::Factor newAlphaBlendDest,
                                      BlendState::Operation newAlphaOperation,
                                      uint8_t newColorMask)
        {
            enableBlending = newEnableBlending;
            colorBlendSource = newColorBlendSource;
            colorBlendDest = newColorBlendDest;
            colorOperation = newColorOperation;
            alphaBlendSource = newAlphaBlendSource;
            alphaBlendDest = newAlphaBlendDest;
            alphaOperation = newAlphaOperation;
            colorMask = newColorMask;

            return true;
        }
    } // namespace graphics
} // namespace ouzel
