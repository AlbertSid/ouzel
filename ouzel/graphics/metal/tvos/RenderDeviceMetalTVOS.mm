// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "core/Setup.h"

#if OUZEL_PLATFORM_TVOS && OUZEL_COMPILE_METAL

#import "core/tvos/DisplayLinkHandler.h"
#include "RenderDeviceMetalTVOS.hpp"
#include "MetalView.h"
#include "core/Window.hpp"
#include "core/tvos/WindowResourceTVOS.hpp"
#include "utils/Log.hpp"

namespace ouzel
{
    namespace graphics
    {
        RenderDeviceMetalTVOS::~RenderDeviceMetalTVOS()
        {
            if (displayLinkHandler) [displayLinkHandler stop];
            flushCommands();
            if (displayLinkHandler) [displayLinkHandler dealloc];
        }

        bool RenderDeviceMetalTVOS::init(Window* newWindow,
                                         const Size2& newSize,
                                         uint32_t newSampleCount,
                                         Texture::Filter newTextureFilter,
                                         uint32_t newMaxAnisotropy,
                                         bool newVerticalSync,
                                         bool newDepth,
                                         bool newDebugRenderer)
        {
            if (!RenderDeviceMetal::init(newWindow,
                                         newSize,
                                         newSampleCount,
                                         newTextureFilter,
                                         newMaxAnisotropy,
                                         newVerticalSync,
                                         newDepth,
                                         newDebugRenderer))
            {
                return false;
            }

            MetalView* view = (MetalView*)static_cast<WindowResourceTVOS*>(newWindow->getResource())->getNativeView();

            metalLayer = (CAMetalLayer*)view.layer;
            metalLayer.device = device;
            CGSize drawableSize = CGSizeMake(newSize.width, newSize.height);
            metalLayer.drawableSize = drawableSize;

            colorFormat = metalLayer.pixelFormat;

            displayLinkHandler = [[DisplayLinkHandler alloc] initWithRenderDevice:this andVerticalSync:verticalSync];

            return true;
        }
    } // namespace graphics
} // namespace ouzel

#endif
