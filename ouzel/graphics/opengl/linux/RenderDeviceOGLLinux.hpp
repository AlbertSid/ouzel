// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "core/Setup.h"

#if OUZEL_PLATFORM_LINUX && OUZEL_COMPILE_OPENGL

#include <atomic>
#define GL_GLEXT_PROTOTYPES 1
#include "GL/glx.h"
#include "graphics/opengl/RenderDeviceOGL.hpp"

namespace ouzel
{
    namespace graphics
    {
        class RenderDeviceOGLLinux: public RenderDeviceOGL
        {
            friend Renderer;
        public:
            virtual ~RenderDeviceOGLLinux();

            virtual std::vector<Size2> getSupportedResolutions() const override;

        private:
            RenderDeviceOGLLinux();

            virtual bool init(Window* newWindow,
                              const Size2& newSize,
                              uint32_t newSampleCount,
                              Texture::Filter newTextureFilter,
                              uint32_t newMaxAnisotropy,
                              bool newVerticalSync,
                              bool newDepth,
                              bool newDebugRenderer) override;

            virtual bool lockContext() override;
            virtual bool swapBuffers() override;
            void main();

            GLXContext context = 0;

            std::atomic<bool> running;
            Thread renderThread;
        };
    } // namespace graphics
} // namespace ouzel

#endif
