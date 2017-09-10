// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "core/CompileConfig.h"

#if OUZEL_PLATFORM_ANDROID && OUZEL_SUPPORTS_OPENGL

#include "RenderDeviceOGLAndroid.hpp"
#include "core/android/WindowAndroid.hpp"
#include "core/Engine.hpp"
#include "utils/Log.hpp"

namespace ouzel
{
    namespace graphics
    {
        RenderDeviceOGLAndroid::~RenderDeviceOGLAndroid()
        {
            running = false;
            flushCommands();
            if (renderThread.joinable()) renderThread.join();

            if (context)
            {
                if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
                {
                    Log(Log::Level::ERR) << "Failed to unset EGL context";
                }

                if (!eglDestroyContext(display, context))
                {
                    Log(Log::Level::ERR) << "Failed to destroy EGL context";
                }
            }

            if (surface)
            {
                if (!eglDestroySurface(display, surface))
                {
                    Log(Log::Level::ERR) << "Failed to destroy EGL surface";
                }
            }

            if (display)
            {
                if (!eglTerminate(display))
                {
                    Log(Log::Level::ERR) << "Failed to terminate EGL";
                }
            }
        }

        bool RenderDeviceOGLAndroid::init(Window* newWindow,
                                          const Size2&,
                                          uint32_t newSampleCount,
                                          Texture::Filter newTextureFilter,
                                          uint32_t newMaxAnisotropy,
                                          bool newVerticalSync,
                                          bool newExclusiveFullscreen,
                                          bool newDepth,
                                          bool newDebugRenderer)
        {
            display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

            if (!display)
            {
                Log(Log::Level::ERR) << "Failed to get display";
                return false;
            }

            if (!eglInitialize(display, nullptr, nullptr))
            {
                Log(Log::Level::ERR) << "Failed to initialize EGL";
                return false;
            }

            const EGLint attributeList[] =
            {
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, newDepth ? 24 : 0,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_SAMPLE_BUFFERS, (newSampleCount > 1) ? 1 : 0,
                EGL_SAMPLES, static_cast<int>(newSampleCount),
                EGL_NONE
            };
            EGLConfig config;
            EGLint numConfig;
            if (!eglChooseConfig(display, attributeList, &config, 1, &numConfig))
            {
                Log(Log::Level::ERR) << "Failed to choose EGL config";
                return false;
            }

            if (!eglBindAPI(EGL_OPENGL_ES_API))
            {
                Log(Log::Level::ERR) << "Failed to bind OpenGL ES API";
                return false;
            }

            EGLint format;
            if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format))
            {
                Log(Log::Level::ERR) << "Failed to get config attribute " << eglGetError();
                return false;
            }

            WindowAndroid* windowAndroid = static_cast<WindowAndroid*>(newWindow);

            ANativeWindow_setBuffersGeometry(windowAndroid->getNativeWindow(), 0, 0, format);

            surface = eglCreateWindowSurface(display, config, windowAndroid->getNativeWindow(), nullptr);
            if (surface == EGL_NO_SURFACE)
            {
                Log(Log::Level::ERR) << "Failed to create EGL window surface";
                return false;
            }

            for (EGLint version = 3; version >= 2; --version)
            {
                std::vector<EGLint> contextAttributes =
                {
                    EGL_CONTEXT_CLIENT_VERSION, version
                };

                if (newDebugRenderer)
                {
                    contextAttributes.push_back(EGL_CONTEXT_FLAGS_KHR);
                    contextAttributes.push_back(EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR);
                }

                contextAttributes.push_back(EGL_NONE);

                context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes.data());

                if (context != EGL_NO_CONTEXT)
                {
                    apiMajorVersion = version;
                    apiMinorVersion = 0;
                    Log(Log::Level::INFO) << "EGL OpenGL ES " << version << " context created";
                    break;
                }
            }

            if (context == EGL_NO_CONTEXT)
            {
                Log(Log::Level::ERR) << "Failed to create EGL context";
                return false;
            }

            if (!eglMakeCurrent(display, surface, surface, context))
            {
                Log(Log::Level::ERR) << "Failed to set current EGL context";
                return false;
            }

            if (!eglSwapInterval(display, newVerticalSync ? 1 : 0))
            {
                Log(Log::Level::ERR) << "Failed to set EGL frame interval";
                return false;
            }

            EGLint surfaceWidth;
            EGLint surfaceHeight;

            if (!eglQuerySurface(display, surface, EGL_WIDTH, &surfaceWidth) ||
                !eglQuerySurface(display, surface, EGL_HEIGHT, &surfaceHeight))
            {
                Log(Log::Level::ERR) << "Failed to get query window size " <<  eglGetError();
                return false;
            }

            frameBufferWidth = surfaceWidth;
            frameBufferHeight = surfaceHeight;

            Size2 backBufferSize = Size2(static_cast<float>(frameBufferWidth),
                                         static_cast<float>(frameBufferHeight));

            newWindow->setSize(backBufferSize / newWindow->getContentScale());

            if (!RenderDeviceOGL::init(newWindow,
                                       backBufferSize,
                                       newSampleCount,
                                       newTextureFilter,
                                       newMaxAnisotropy,
                                       newVerticalSync,
                                       newExclusiveFullscreen,
                                       newDepth,
                                       newDebugRenderer))
            {
                return false;
            }

            if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
            {
                Log(Log::Level::ERR) << "Failed to unset EGL context";
            }

            running = true;
            renderThread = std::thread(&RenderDeviceOGLAndroid::main, this);

            return true;
        }

        bool RenderDeviceOGLAndroid::reload()
        {
            running = false;
            flushCommands();
            if (renderThread.joinable()) renderThread.join();

            const EGLint attributeList[] =
            {
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, depth ? 24 : 0,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_SAMPLE_BUFFERS, (sampleCount > 1) ? 1 : 0,
                EGL_SAMPLES, static_cast<int>(sampleCount),
                EGL_NONE
            };
            EGLConfig config;
            EGLint numConfig;
            if (!eglChooseConfig(display, attributeList, &config, 1, &numConfig))
            {
                Log(Log::Level::ERR) << "Failed to choose EGL config";
                return false;
            }

            if (!eglBindAPI(EGL_OPENGL_ES_API))
            {
                Log(Log::Level::ERR) << "Failed to bind OpenGL ES API";
                return false;
            }

            EGLint format;
            if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format))
            {
                Log(Log::Level::ERR) << "Failed to get config attribute " << eglGetError();
                return false;
            }

            WindowAndroid* windowAndroid = static_cast<WindowAndroid*>(sharedEngine->getWindow());

            ANativeWindow_setBuffersGeometry(windowAndroid->getNativeWindow(), 0, 0, format);

            surface = eglCreateWindowSurface(display, config, windowAndroid->getNativeWindow(), nullptr);
            if (surface == EGL_NO_SURFACE)
            {
                Log(Log::Level::ERR) << "Failed to create EGL window surface";
                return false;
            }

            for (EGLint version = 3; version >= 2; --version)
            {
                std::vector<EGLint> contextAttributes =
                {
                    EGL_CONTEXT_CLIENT_VERSION, version
                };

                if (debugRenderer)
                {
                    contextAttributes.push_back(EGL_CONTEXT_FLAGS_KHR);
                    contextAttributes.push_back(EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR);
                }

                contextAttributes.push_back(EGL_NONE);

                context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes.data());

                if (context != EGL_NO_CONTEXT)
                {
                    apiMajorVersion = version;
                    apiMinorVersion = 0;
                    Log(Log::Level::INFO) << "EGL OpenGL ES " << version << " context created";
                    break;
                }
            }

            if (context == EGL_NO_CONTEXT)
            {
                Log(Log::Level::ERR) << "Failed to create EGL context";
                return false;
            }

            if (!eglMakeCurrent(display, surface, surface, context))
            {
                Log(Log::Level::ERR) << "Failed to set current EGL context";
                return false;
            }

            if (!eglSwapInterval(display, verticalSync ? 1 : 0))
            {
                Log(Log::Level::ERR) << "Failed to set EGL frame interval";
                return false;
            }

            EGLint surfaceWidth;
            EGLint surfaceHeight;

            if (!eglQuerySurface(display, surface, EGL_WIDTH, &surfaceWidth) ||
                !eglQuerySurface(display, surface, EGL_HEIGHT, &surfaceHeight))
            {
                Log(Log::Level::ERR) << "Failed to get query window size " <<  eglGetError();
                return false;
            }

            frameBufferWidth = surfaceWidth;
            frameBufferHeight = surfaceHeight;

            Size2 backBufferSize = Size2(static_cast<float>(frameBufferWidth),
                                         static_cast<float>(frameBufferHeight));

            windowAndroid->setSize(backBufferSize / windowAndroid->getContentScale());

            stateCache = StateCache();

            glDisable(GL_DITHER);
            glDepthFunc(GL_LEQUAL);

            if (checkOpenGLError())
            {
                Log(Log::Level::ERR) << "Failed to set depth function";
                return false;
            }

            {
                std::lock_guard<std::mutex> lock(resourceMutex);

                for (const std::unique_ptr<Resource>& resource : resources)
                {
                    if (!resource->reload())
                    {
                        return false;
                    }
                }
            }

            if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
            {
                Log(Log::Level::ERR) << "Failed to unset EGL context";
            }

            running = true;
            renderThread = std::thread(&RenderDeviceOGLAndroid::main, this);

            return true;
        }

        bool RenderDeviceOGLAndroid::destroy()
        {
            running = false;
            flushCommands();
            if (renderThread.joinable()) renderThread.join();

            if (context)
            {
                if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
                {
                    Log(Log::Level::ERR) << "Failed to unset EGL context";
                }

                if (!eglDestroyContext(display, context))
                {
                    Log(Log::Level::ERR) << "Failed to destroy EGL context";
                }

                context = nullptr;
            }

            if (surface)
            {
                if (!eglDestroySurface(display, surface))
                {
                    Log(Log::Level::ERR) << "Failed to destroy EGL surface";
                }

                surface = nullptr;
            }

            return true;
        }

        bool RenderDeviceOGLAndroid::lockContext()
        {
            if (!eglMakeCurrent(display, surface, surface, context))
            {
                Log(Log::Level::ERR) << "Failed to set current EGL context, error: " << eglGetError();
                return false;
            }

            return true;
        }

        bool RenderDeviceOGLAndroid::swapBuffers()
        {
            if (eglSwapBuffers(display, surface) != EGL_TRUE)
            {
                Log(Log::Level::ERR) << "Failed to swap buffers " << eglGetError();
                return false;
            }

            return true;
        }

        void RenderDeviceOGLAndroid::main()
        {
            while (running)
            {
                process();
            }

            if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
            {
                Log(Log::Level::ERR) << "Failed to unset EGL context, error: " << eglGetError();
            }
        }
    } // namespace graphics
} // namespace ouzel

#endif
