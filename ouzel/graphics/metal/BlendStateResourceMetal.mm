// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "core/Setup.h"

#if OUZEL_COMPILE_METAL

#include "BlendStateResourceMetal.hpp"

namespace ouzel
{
    namespace graphics
    {
        BlendStateResourceMetal::BlendStateResourceMetal()
        {
        }

        static MTLBlendFactor getBlendFactor(BlendState::Factor blendFactor)
        {
            switch (blendFactor)
            {
                case BlendState::Factor::ZERO: return MTLBlendFactorZero;
                case BlendState::Factor::ONE: return MTLBlendFactorOne;
                case BlendState::Factor::SRC_COLOR: return MTLBlendFactorSourceColor;
                case BlendState::Factor::INV_SRC_COLOR: return MTLBlendFactorOneMinusSourceColor;
                case BlendState::Factor::SRC_ALPHA: return MTLBlendFactorSourceAlpha;
                case BlendState::Factor::INV_SRC_ALPHA: return MTLBlendFactorOneMinusSourceAlpha;
                case BlendState::Factor::DEST_ALPHA: return MTLBlendFactorDestinationAlpha;
                case BlendState::Factor::INV_DEST_ALPHA: return MTLBlendFactorOneMinusDestinationAlpha;
                case BlendState::Factor::DEST_COLOR: return MTLBlendFactorDestinationColor;
                case BlendState::Factor::INV_DEST_COLOR: return MTLBlendFactorOneMinusDestinationColor;
                case BlendState::Factor::SRC_ALPHA_SAT: return MTLBlendFactorSourceAlphaSaturated;
                case BlendState::Factor::BLEND_FACTOR: return MTLBlendFactorBlendColor;
                case BlendState::Factor::INV_BLEND_FACTOR: return MTLBlendFactorOneMinusBlendColor;
            }

            return MTLBlendFactorZero;
        }

        static MTLBlendOperation getBlendOperation(BlendState::Operation blendOperation)
        {
            switch (blendOperation)
            {
                case BlendState::Operation::ADD: return MTLBlendOperationAdd;
                case BlendState::Operation::SUBTRACT: return MTLBlendOperationSubtract;
                case BlendState::Operation::REV_SUBTRACT: return MTLBlendOperationReverseSubtract;
                case BlendState::Operation::MIN: return MTLBlendOperationMin;
                case BlendState::Operation::MAX: return MTLBlendOperationMax;
            }

            return MTLBlendOperationAdd;
        }

        bool BlendStateResourceMetal::init(bool newEnableBlending,
                                           BlendState::Factor newColorBlendSource, BlendState::Factor newColorBlendDest,
                                           BlendState::Operation newColorOperation,
                                           BlendState::Factor newAlphaBlendSource, BlendState::Factor newAlphaBlendDest,
                                           BlendState::Operation newAlphaOperation,
                                           uint8_t newColorMask)
        {
            if (!BlendStateResource::init(newEnableBlending,
                                          newColorBlendSource, newColorBlendDest,
                                          newColorOperation,
                                          newAlphaBlendSource, newAlphaBlendDest,
                                          newAlphaOperation,
                                          newColorMask))
            {
                return false;
            }

            rgbBlendOperation = getBlendOperation(colorOperation);
            alphaBlendOperation = getBlendOperation(alphaOperation);
            sourceRGBBlendFactor = getBlendFactor(colorBlendSource);
            destinationRGBBlendFactor = getBlendFactor(colorBlendDest);
            sourceAlphaBlendFactor = getBlendFactor(alphaBlendSource);
            destinationAlphaBlendFactor = getBlendFactor(alphaBlendDest);
            metalBlendingEnabled = enableBlending;

            colorWriteMask = MTLColorWriteMaskNone;
            if (colorMask & BlendState::COLOR_MASK_RED) colorWriteMask |= MTLColorWriteMaskRed;
            if (colorMask & BlendState::COLOR_MASK_GREEN) colorWriteMask |= MTLColorWriteMaskGreen;
            if (colorMask & BlendState::COLOR_MASK_BLUE) colorWriteMask |= MTLColorWriteMaskBlue;
            if (colorMask & BlendState::COLOR_MASK_ALPHA) colorWriteMask |= MTLColorWriteMaskAlpha;

            return true;
        }
    } // namespace graphics
} // namespace ouzel

#endif
