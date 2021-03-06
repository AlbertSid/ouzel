// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#endif
#include <algorithm>
#include <stdexcept>
#include "Setup.h"
#include "Engine.hpp"
#include "utils/Utils.hpp"
#include "graphics/Renderer.hpp"
#include "audio/Audio.hpp"

#if OUZEL_COMPILE_OPENGL
#  include "graphics/opengl/OGL.h"
#  if OUZEL_OPENGLES
#    include "opengl/ColorPSGLES2.h"
#    include "opengl/ColorVSGLES2.h"
#    include "opengl/TexturePSGLES2.h"
#    include "opengl/TextureVSGLES2.h"
#    include "opengl/ColorPSGLES3.h"
#    include "opengl/ColorVSGLES3.h"
#    include "opengl/TexturePSGLES3.h"
#    include "opengl/TextureVSGLES3.h"
#  else
#    include "opengl/ColorPSGL2.h"
#    include "opengl/ColorVSGL2.h"
#    include "opengl/TexturePSGL2.h"
#    include "opengl/TextureVSGL2.h"
#    include "opengl/ColorPSGL3.h"
#    include "opengl/ColorVSGL3.h"
#    include "opengl/TexturePSGL3.h"
#    include "opengl/TextureVSGL3.h"
#    include "opengl/ColorPSGL4.h"
#    include "opengl/ColorVSGL4.h"
#    include "opengl/TexturePSGL4.h"
#    include "opengl/TextureVSGL4.h"
#  endif
#endif

#if OUZEL_COMPILE_DIRECT3D11
#  include "direct3d11/TexturePSD3D11.h"
#  include "direct3d11/TextureVSD3D11.h"
#  include "direct3d11/ColorPSD3D11.h"
#  include "direct3d11/ColorVSD3D11.h"
#endif

#if OUZEL_COMPILE_METAL
#  if TARGET_OS_IOS
#    include "metal/ColorPSIOS.h"
#    include "metal/ColorVSIOS.h"
#    include "metal/TexturePSIOS.h"
#    include "metal/TextureVSIOS.h"
#    define COLOR_PIXEL_SHADER_METAL ColorPSIOS_metallib
#    define COLOR_VERTEX_SHADER_METAL ColorVSIOS_metallib
#    define TEXTURE_PIXEL_SHADER_METAL TexturePSIOS_metallib
#    define TEXTURE_VERTEX_SHADER_METAL TextureVSIOS_metallib
#  elif TARGET_OS_TV
#    include "metal/ColorPSTVOS.h"
#    include "metal/ColorVSTVOS.h"
#    include "metal/TexturePSTVOS.h"
#    include "metal/TextureVSTVOS.h"
#    define COLOR_PIXEL_SHADER_METAL ColorPSTVOS_metallib
#    define COLOR_VERTEX_SHADER_METAL ColorVSTVOS_metallib
#    define TEXTURE_PIXEL_SHADER_METAL TexturePSTVOS_metallib
#    define TEXTURE_VERTEX_SHADER_METAL TextureVSTVOS_metallib
#  elif TARGET_OS_MAC
#    include "metal/ColorPSMacOS.h"
#    include "metal/ColorVSMacOS.h"
#    include "metal/TexturePSMacOS.h"
#    include "metal/TextureVSMacOS.h"
#    define COLOR_PIXEL_SHADER_METAL ColorPSMacOS_metallib
#    define COLOR_VERTEX_SHADER_METAL ColorVSMacOS_metallib
#    define TEXTURE_PIXEL_SHADER_METAL TexturePSMacOS_metallib
#    define TEXTURE_VERTEX_SHADER_METAL TextureVSMacOS_metallib
#  endif
#endif

namespace ouzel
{
    Engine* engine = nullptr;

    Engine::Engine():
        fileSystem(*this),
        assetBundle(cache, fileSystem)
    {
        engine = this;
    }

    Engine::~Engine()
    {
        if (active)
        {
            auto event = std::make_unique<SystemEvent>();
            event->type = Event::Type::EngineStop;
            eventDispatcher.postEvent(std::move(event));
        }

        paused = true;
        active = false;

#if !defined(__EMSCRIPTEN__)
        if (updateThread.isJoinable())
        {
            std::unique_lock<std::mutex> lock(updateMutex);
            updateCondition.notify_all();
            lock.unlock();
            updateThread.join();
        }
#endif
    }

    void Engine::init()
    {
        Thread::setCurrentThreadName("Main");

        Size2U size;
        uint32_t sampleCount = 1; // MSAA sample count
        graphics::SamplerFilter textureFilter = graphics::SamplerFilter::Point;
        uint32_t maxAnisotropy = 1;
        bool resizable = false;
        bool fullscreen = false;
        bool verticalSync = true;
        bool depth = false;
        bool stencil = false;
        bool debugRenderer = false;
        bool exclusiveFullscreen = false;
        bool highDpi = true; // should high DPI resolution be used
        bool debugAudio = false;

        defaultSettings = ini::Data(fileSystem.readFile("settings.ini"));

        try
        {
            userSettings = ini::Data(fileSystem.readFile(fileSystem.getStorageDirectory() + storage::FileSystem::DIRECTORY_SEPARATOR + "settings.ini"));
        }
        catch (const std::exception&)
        {
            log(Log::Level::Info) << "User settings not provided";
        }

        const ini::Section& userEngineSection = userSettings.getSection("engine");
        const ini::Section& defaultEngineSection = defaultSettings.getSection("engine");

        std::string graphicsDriverValue = userEngineSection.getValue("graphicsDriver", defaultEngineSection.getValue("graphicsDriver"));

        std::string widthValue = userEngineSection.getValue("width", defaultEngineSection.getValue("width"));
        if (!widthValue.empty()) size.v[0] = static_cast<uint32_t>(std::stoul(widthValue));

        std::string heightValue = userEngineSection.getValue("height", defaultEngineSection.getValue("height"));
        if (!heightValue.empty()) size.v[1] = static_cast<uint32_t>(std::stoul(heightValue));

        std::string sampleCountValue = userEngineSection.getValue("sampleCount", defaultEngineSection.getValue("sampleCount"));
        if (!sampleCountValue.empty()) sampleCount = static_cast<uint32_t>(std::stoul(sampleCountValue));

        std::string textureFilterValue = userEngineSection.getValue("textureFilter", defaultEngineSection.getValue("textureFilter"));
        if (!textureFilterValue.empty())
        {
            if (textureFilterValue == "point")
                textureFilter = graphics::SamplerFilter::Point;
            else if (textureFilterValue == "linear")
                textureFilter = graphics::SamplerFilter::Linear;
            else if (textureFilterValue == "bilinear")
                textureFilter = graphics::SamplerFilter::Bilinear;
            else if (textureFilterValue == "trilinear")
                textureFilter = graphics::SamplerFilter::Trilinear;
            else
                throw std::runtime_error("Invalid texture filter specified");
        }

        std::string maxAnisotropyValue = userEngineSection.getValue("maxAnisotropy", defaultEngineSection.getValue("maxAnisotropy"));
        if (!maxAnisotropyValue.empty()) maxAnisotropy = static_cast<uint32_t>(std::stoul(maxAnisotropyValue));

        std::string resizableValue = userEngineSection.getValue("resizable", defaultEngineSection.getValue("resizable"));
        if (!resizableValue.empty()) resizable = (resizableValue == "true" || resizableValue == "1" || resizableValue == "yes");

        std::string fullscreenValue = userEngineSection.getValue("fullscreen", defaultEngineSection.getValue("fullscreen"));
        if (!fullscreenValue.empty()) fullscreen = (fullscreenValue == "true" || fullscreenValue == "1" || fullscreenValue == "yes");

        std::string verticalSyncValue = userEngineSection.getValue("verticalSync", defaultEngineSection.getValue("verticalSync"));
        if (!verticalSyncValue.empty()) verticalSync = (verticalSyncValue == "true" || verticalSyncValue == "1" || verticalSyncValue == "yes");

        std::string exclusiveFullscreenValue = userEngineSection.getValue("exclusiveFullscreen", defaultEngineSection.getValue("exclusiveFullscreen"));
        if (!exclusiveFullscreenValue.empty()) exclusiveFullscreen = (exclusiveFullscreenValue == "true" || exclusiveFullscreenValue == "1" || exclusiveFullscreenValue == "yes");

        std::string depthValue = userEngineSection.getValue("depth", defaultEngineSection.getValue("depth"));
        if (!depthValue.empty()) depth = (depthValue == "true" || depthValue == "1" || depthValue == "yes");

        std::string stencilValue = userEngineSection.getValue("stencil", defaultEngineSection.getValue("stencil"));
        if (!stencilValue.empty()) stencil = (depthValue == "true" || depthValue == "1" || depthValue == "yes");

        std::string debugRendererValue = userEngineSection.getValue("debugRenderer", defaultEngineSection.getValue("debugRenderer"));
        if (!debugRendererValue.empty()) debugRenderer = (debugRendererValue == "true" || debugRendererValue == "1" || debugRendererValue == "yes");

        std::string highDpiValue = userEngineSection.getValue("highDpi", defaultEngineSection.getValue("highDpi"));
        if (!highDpiValue.empty()) highDpi = (highDpiValue == "true" || highDpiValue == "1" || highDpiValue == "yes");

        std::string audioDriverValue = userEngineSection.getValue("audioDriver", defaultEngineSection.getValue("audioDriver"));

        std::string debugAudioValue = userEngineSection.getValue("debugAudio", defaultEngineSection.getValue("debugAudio"));
        if (!debugAudioValue.empty()) debugAudio = (debugAudioValue == "true" || debugAudioValue == "1" || debugAudioValue == "yes");

        graphics::Driver graphicsDriver = graphics::Renderer::getDriver(graphicsDriverValue);

        const uint32_t windowFlags = (resizable ? Window::Flags::Resizable : 0) |
            (fullscreen ? Window::Flags::Fullscreen : 0) |
            (exclusiveFullscreen ? Window::Flags::ExclusiveFullscreen : 0) |
            (highDpi ? Window::Flags::HighDpi : 0) |
            (depth ? Window::Flags::Depth : 0);

        window = std::make_unique<Window>(*this,
                                          size,
                                          windowFlags,
                                          OUZEL_APPLICATION_NAME,
                                          graphicsDriver);

        renderer = std::make_unique<graphics::Renderer>(graphicsDriver,
                                                        window.get(),
                                                        window->getResolution(),
                                                        sampleCount,
                                                        textureFilter,
                                                        maxAnisotropy,
                                                        false,
                                                        verticalSync,
                                                        depth,
                                                        stencil,
                                                        debugRenderer);

        audio::Driver audioDriver = audio::Audio::getDriver(audioDriverValue);
        audio = std::make_unique<audio::Audio>(audioDriver, debugAudio);

        inputManager = std::make_unique<input::InputManager>();

        // default assets
        switch (graphicsDriver)
        {
#if OUZEL_COMPILE_OPENGL
            case graphics::Driver::OpenGL:
            {
                std::unique_ptr<graphics::Shader> textureShader;

                switch (renderer->getDevice()->getAPIMajorVersion())
                {
#  if OUZEL_OPENGLES
                    case 2:
                        textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                           std::vector<uint8_t>(std::begin(TexturePSGLES2_glsl),
                                                                                                std::end(TexturePSGLES2_glsl)),
                                                                           std::vector<uint8_t>(std::begin(TextureVSGLES2_glsl),
                                                                                                std::end(TextureVSGLES2_glsl)),
                                                                           std::set<graphics::Vertex::Attribute::Usage>{
                                                                               graphics::Vertex::Attribute::Usage::Position,
                                                                               graphics::Vertex::Attribute::Usage::Color,
                                                                               graphics::Vertex::Attribute::Usage::TextureCoordinates0
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"color",graphics::DataType::FloatVector4}
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                           });
                        break;
                    case 3:
                        textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                           std::vector<uint8_t>(std::begin(TexturePSGLES3_glsl),
                                                                                                std::end(TexturePSGLES3_glsl)),
                                                                           std::vector<uint8_t>(std::begin(TextureVSGLES3_glsl),
                                                                                                std::end(TextureVSGLES3_glsl)),
                                                                           std::set<graphics::Vertex::Attribute::Usage>{
                                                                               graphics::Vertex::Attribute::Usage::Position,
                                                                               graphics::Vertex::Attribute::Usage::Color,
                                                                               graphics::Vertex::Attribute::Usage::TextureCoordinates0
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"color", graphics::DataType::FloatVector4}
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                           });
                        break;
#  else
                    case 2:
                        textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                           std::vector<uint8_t>(std::begin(TexturePSGL2_glsl),
                                                                                                std::end(TexturePSGL2_glsl)),
                                                                           std::vector<uint8_t>(std::begin(TextureVSGL2_glsl),
                                                                                                std::end(TextureVSGL2_glsl)),
                                                                           std::set<graphics::Vertex::Attribute::Usage>{
                                                                               graphics::Vertex::Attribute::Usage::Position,
                                                                               graphics::Vertex::Attribute::Usage::Color,
                                                                               graphics::Vertex::Attribute::Usage::TextureCoordinates0
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"color", graphics::DataType::FloatVector4}
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                           });
                        break;
                    case 3:
                        textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                           std::vector<uint8_t>(std::begin(TexturePSGL3_glsl),
                                                                                                std::end(TexturePSGL3_glsl)),
                                                                           std::vector<uint8_t>(std::begin(TextureVSGL3_glsl),
                                                                                                std::end(TextureVSGL3_glsl)),
                                                                           std::set<graphics::Vertex::Attribute::Usage>{
                                                                               graphics::Vertex::Attribute::Usage::Position,
                                                                               graphics::Vertex::Attribute::Usage::Color,
                                                                               graphics::Vertex::Attribute::Usage::TextureCoordinates0
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"color", graphics::DataType::FloatVector4}
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                           });
                        break;
                    case 4:
                        textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                           std::vector<uint8_t>(std::begin(TexturePSGL4_glsl),
                                                                                                std::end(TexturePSGL4_glsl)),
                                                                           std::vector<uint8_t>(std::begin(TextureVSGL4_glsl),
                                                                                                std::end(TextureVSGL4_glsl)),
                                                                           std::set<graphics::Vertex::Attribute::Usage>{
                                                                               graphics::Vertex::Attribute::Usage::Position,
                                                                               graphics::Vertex::Attribute::Usage::Color,
                                                                               graphics::Vertex::Attribute::Usage::TextureCoordinates0
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"color", graphics::DataType::FloatVector4}
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                           });
                        break;
#  endif
                    default:
                        throw std::runtime_error("Unsupported OpenGL version");
                }

                assetBundle.setShader(SHADER_TEXTURE, std::move(textureShader));

                auto colorShader = std::make_unique<graphics::Shader>(*renderer);

                switch (renderer->getDevice()->getAPIMajorVersion())
                {
#  if OUZEL_OPENGLES
                    case 2:
                        colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                         std::vector<uint8_t>(std::begin(ColorPSGLES2_glsl),
                                                                                              std::end(ColorPSGLES2_glsl)),
                                                                         std::vector<uint8_t>(std::begin(ColorVSGLES2_glsl),
                                                                                              std::end(ColorVSGLES2_glsl)),
                                                                         std::set<graphics::Vertex::Attribute::Usage>{
                                                                             graphics::Vertex::Attribute::Usage::Position,
                                                                             graphics::Vertex::Attribute::Usage::Color
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"color", graphics::DataType::FloatVector4}
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                         });

                        break;
                    case 3:
                        colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                         std::vector<uint8_t>(std::begin(ColorPSGLES3_glsl),
                                                                                              std::end(ColorPSGLES3_glsl)),
                                                                         std::vector<uint8_t>(std::begin(ColorVSGLES3_glsl),
                                                                                              std::end(ColorVSGLES3_glsl)),
                                                                         std::set<graphics::Vertex::Attribute::Usage>{
                                                                             graphics::Vertex::Attribute::Usage::Position,
                                                                             graphics::Vertex::Attribute::Usage::Color
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"color", graphics::DataType::FloatVector4}
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                         });
                        break;
#  else
                    case 2:
                        colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                         std::vector<uint8_t>(std::begin(ColorPSGL2_glsl),
                                                                                              std::end(ColorPSGL2_glsl)),
                                                                         std::vector<uint8_t>(std::begin(ColorVSGL2_glsl),
                                                                                              std::end(ColorVSGL2_glsl)),
                                                                         std::set<graphics::Vertex::Attribute::Usage>{
                                                                             graphics::Vertex::Attribute::Usage::Position,
                                                                             graphics::Vertex::Attribute::Usage::Color
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"color", graphics::DataType::FloatVector4}
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                         });
                        break;
                    case 3:
                        colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                         std::vector<uint8_t>(std::begin(ColorPSGL3_glsl),
                                                                                              std::end(ColorPSGL3_glsl)),
                                                                         std::vector<uint8_t>(std::begin(ColorVSGL3_glsl),
                                                                                              std::end(ColorVSGL3_glsl)),
                                                                         std::set<graphics::Vertex::Attribute::Usage>{
                                                                             graphics::Vertex::Attribute::Usage::Position,
                                                                             graphics::Vertex::Attribute::Usage::Color
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"color", graphics::DataType::FloatVector4}
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                         });
                        break;
                    case 4:
                        colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                         std::vector<uint8_t>(std::begin(ColorPSGL4_glsl),
                                                                                              std::end(ColorPSGL4_glsl)),
                                                                         std::vector<uint8_t>(std::begin(ColorVSGL4_glsl),
                                                                                              std::end(ColorVSGL4_glsl)),
                                                                         std::set<graphics::Vertex::Attribute::Usage>{
                                                                             graphics::Vertex::Attribute::Usage::Position,
                                                                             graphics::Vertex::Attribute::Usage::Color
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"color", graphics::DataType::FloatVector4}
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                         });
                        break;
#  endif
                    default:
                        throw std::runtime_error("Unsupported OpenGL version");
                }

                assetBundle.setShader(SHADER_COLOR, std::move(colorShader));
                break;
            }
#endif

#if OUZEL_COMPILE_DIRECT3D11
            case graphics::Driver::Direct3D11:
            {
                auto textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                        std::vector<uint8_t>(std::begin(TEXTURE_PIXEL_SHADER_D3D11),
                                                                                             std::end(TEXTURE_PIXEL_SHADER_D3D11)),
                                                                        std::vector<uint8_t>(std::begin(TEXTURE_VERTEX_SHADER_D3D11),
                                                                                             std::end(TEXTURE_VERTEX_SHADER_D3D11)),
                                                                        std::set<graphics::Vertex::Attribute::Usage>{
                                                                            graphics::Vertex::Attribute::Usage::Position,
                                                                            graphics::Vertex::Attribute::Usage::Color,
                                                                            graphics::Vertex::Attribute::Usage::TextureCoordinates0
                                                                        },
                                                                        std::vector<std::pair<std::string, graphics::DataType>>{
                                                                            {"color", graphics::DataType::FloatVector4}
                                                                        },
                                                                        std::vector<std::pair<std::string, graphics::DataType>>{
                                                                            {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                        });

                assetBundle.setShader(SHADER_TEXTURE, std::move(textureShader));

                auto colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                      std::vector<uint8_t>(std::begin(COLOR_PIXEL_SHADER_D3D11),
                                                                                           std::end(COLOR_PIXEL_SHADER_D3D11)),
                                                                      std::vector<uint8_t>(std::begin(COLOR_VERTEX_SHADER_D3D11),
                                                                                           std::end(COLOR_VERTEX_SHADER_D3D11)),
                                                                      std::set<graphics::Vertex::Attribute::Usage>{
                                                                          graphics::Vertex::Attribute::Usage::Position,
                                                                          graphics::Vertex::Attribute::Usage::Color
                                                                      },
                                                                      std::vector<std::pair<std::string, graphics::DataType>>{
                                                                          {"color", graphics::DataType::FloatVector4}
                                                                      },
                                                                      std::vector<std::pair<std::string, graphics::DataType>>{
                                                                          {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                      });

                assetBundle.setShader(SHADER_COLOR, std::move(colorShader));
                break;
            }
#endif

#if OUZEL_COMPILE_METAL
            case graphics::Driver::Metal:
            {
                auto textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                        std::vector<uint8_t>(std::begin(TEXTURE_PIXEL_SHADER_METAL),
                                                                                             std::end(TEXTURE_PIXEL_SHADER_METAL)),
                                                                        std::vector<uint8_t>(std::begin(TEXTURE_VERTEX_SHADER_METAL),
                                                                                             std::end(TEXTURE_VERTEX_SHADER_METAL)),
                                                                        std::set<graphics::Vertex::Attribute::Usage>{
                                                                            graphics::Vertex::Attribute::Usage::Position,
                                                                            graphics::Vertex::Attribute::Usage::Color,
                                                                            graphics::Vertex::Attribute::Usage::TextureCoordinates0
                                                                        },
                                                                        std::vector<std::pair<std::string, graphics::DataType>>{
                                                                            {"color", graphics::DataType::FloatVector4}
                                                                        },
                                                                        std::vector<std::pair<std::string, graphics::DataType>>{
                                                                            {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                        },
                                                                        256, 256,
                                                                        "mainPS", "mainVS");

                assetBundle.setShader(SHADER_TEXTURE, std::move(textureShader));

                auto colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                      std::vector<uint8_t>(std::begin(COLOR_PIXEL_SHADER_METAL),
                                                                                           std::end(COLOR_PIXEL_SHADER_METAL)),
                                                                      std::vector<uint8_t>(std::begin(COLOR_VERTEX_SHADER_METAL),
                                                                                           std::end(COLOR_VERTEX_SHADER_METAL)),
                                                                      std::set<graphics::Vertex::Attribute::Usage>{
                                                                          graphics::Vertex::Attribute::Usage::Position,
                                                                          graphics::Vertex::Attribute::Usage::Color
                                                                      },
                                                                      std::vector<std::pair<std::string, graphics::DataType>>{
                                                                          {"color", graphics::DataType::FloatVector4}
                                                                      },
                                                                      std::vector<std::pair<std::string, graphics::DataType>>{
                                                                          {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                      },
                                                                      256, 256,
                                                                      "mainPS", "mainVS");

                assetBundle.setShader(SHADER_COLOR, std::move(colorShader));
                break;
            }
#endif

            default:
            {
                auto textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                        std::vector<uint8_t>(),
                                                                        std::vector<uint8_t>(),
                                                                        std::set<graphics::Vertex::Attribute::Usage>{
                                                                            graphics::Vertex::Attribute::Usage::Position,
                                                                            graphics::Vertex::Attribute::Usage::Color,
                                                                            graphics::Vertex::Attribute::Usage::TextureCoordinates0
                                                                        },
                                                                        std::vector<std::pair<std::string, graphics::DataType>>{
                                                                            {"color", graphics::DataType::FloatVector4}
                                                                        },
                                                                        std::vector<std::pair<std::string, graphics::DataType>>{
                                                                            {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                        });

                assetBundle.setShader(SHADER_TEXTURE, std::move(textureShader));

                auto colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                      std::vector<uint8_t>(),
                                                                      std::vector<uint8_t>(),
                                                                      std::set<graphics::Vertex::Attribute::Usage>{
                                                                          graphics::Vertex::Attribute::Usage::Position,
                                                                          graphics::Vertex::Attribute::Usage::Color
                                                                      },
                                                                      std::vector<std::pair<std::string, graphics::DataType>>{
                                                                          {"color", graphics::DataType::FloatVector4}
                                                                      },
                                                                      std::vector<std::pair<std::string, graphics::DataType>>{
                                                                          {"modelViewProj", graphics::DataType::FloatMatrix4}
                                                                      });

                assetBundle.setShader(SHADER_COLOR, std::move(colorShader));
                break;
            }
        }

        auto noBlendState = std::make_unique<graphics::BlendState>(*renderer,
                                                                   false,
                                                                   graphics::BlendFactor::One,
                                                                   graphics::BlendFactor::Zero,
                                                                   graphics::BlendOperation::Add,
                                                                   graphics::BlendFactor::One,
                                                                   graphics::BlendFactor::Zero,
                                                                   graphics::BlendOperation::Add);

        assetBundle.setBlendState(BLEND_NO_BLEND, std::move(noBlendState));

        auto addBlendState = std::make_unique<graphics::BlendState>(*renderer,
                                                                    true,
                                                                    graphics::BlendFactor::One,
                                                                    graphics::BlendFactor::One,
                                                                    graphics::BlendOperation::Add,
                                                                    graphics::BlendFactor::One,
                                                                    graphics::BlendFactor::One,
                                                                    graphics::BlendOperation::Add);

        assetBundle.setBlendState(BLEND_ADD, std::move(addBlendState));

        auto multiplyBlendState = std::make_unique<graphics::BlendState>(*renderer,
                                                                         true,
                                                                         graphics::BlendFactor::DestColor,
                                                                         graphics::BlendFactor::Zero,
                                                                         graphics::BlendOperation::Add,
                                                                         graphics::BlendFactor::One,
                                                                         graphics::BlendFactor::One,
                                                                         graphics::BlendOperation::Add);

        assetBundle.setBlendState(BLEND_MULTIPLY, std::move(multiplyBlendState));

        auto alphaBlendState = std::make_unique<graphics::BlendState>(*renderer,
                                                                      true,
                                                                      graphics::BlendFactor::SrcAlpha,
                                                                      graphics::BlendFactor::InvSrcAlpha,
                                                                      graphics::BlendOperation::Add,
                                                                      graphics::BlendFactor::One,
                                                                      graphics::BlendFactor::One,
                                                                      graphics::BlendOperation::Add);

        assetBundle.setBlendState(BLEND_ALPHA, std::move(alphaBlendState));

        auto screenBlendState = std::make_unique<graphics::BlendState>(*renderer,
                                                                       true,
                                                                       graphics::BlendFactor::One,
                                                                       graphics::BlendFactor::InvSrcAlpha,
                                                                       graphics::BlendOperation::Add,
                                                                       graphics::BlendFactor::One,
                                                                       graphics::BlendFactor::One,
                                                                       graphics::BlendOperation::Add);

        assetBundle.setBlendState(BLEND_SCREEN, std::move(screenBlendState));

        auto whitePixelTexture = std::make_shared<graphics::Texture>(*renderer,
                                                                     std::vector<uint8_t>{255, 255, 255, 255},
                                                                     Size2U(1, 1),
                                                                     0, 1);
        assetBundle.setTexture(TEXTURE_WHITE_PIXEL, whitePixelTexture);
    }

    void Engine::start()
    {
        if (!active)
        {
            auto event = std::make_unique<SystemEvent>();
            event->type = Event::Type::EngineStart;
            eventDispatcher.postEvent(std::move(event));

            active = true;
            paused = false;

#if !defined(__EMSCRIPTEN__)
            updateThread = Thread(&Engine::engineMain, this);
#else
            main();
#endif
        }
    }

    void Engine::pause()
    {
        if (active && !paused)
        {
            auto event = std::make_unique<SystemEvent>();
            event->type = Event::Type::EnginePause;
            eventDispatcher.postEvent(std::move(event));

            paused = true;
        }
    }

    void Engine::resume()
    {
        if (active && paused)
        {
            auto event = std::make_unique<SystemEvent>();
            event->type = Event::Type::EngineResume;
            eventDispatcher.postEvent(std::move(event));

            paused = false;

#if !defined(__EMSCRIPTEN__)
            updateCondition.notify_all();
#endif
        }
    }

    void Engine::exit()
    {
        paused = true;

        if (active)
        {
            auto event = std::make_unique<SystemEvent>();
            event->type = Event::Type::EngineStop;
            eventDispatcher.postEvent(std::move(event));

            active = false;
        }

#if !defined(__EMSCRIPTEN__)
        if (updateThread.isJoinable() &&
            updateThread.getId() != std::this_thread::get_id())
        {
            std::unique_lock<std::mutex> lock(updateMutex);
            updateCondition.notify_all();
            lock.unlock();
            updateThread.join();
        }
#endif
    }

    void Engine::update()
    {
        eventDispatcher.dispatchEvents();

        auto currentTime = std::chrono::steady_clock::now();
        auto diff = currentTime - previousUpdateTime;

        if (diff > std::chrono::milliseconds(1)) // at least one millisecond has passed
        {
            if (diff > std::chrono::seconds(1000 / 20)) diff = std::chrono::milliseconds(1000 / 20); // limit the update rate to a minimum 20 FPS

            previousUpdateTime = currentTime;
            const float delta = std::chrono::duration_cast<std::chrono::microseconds>(diff).count() / 1000000.0F;

            auto updateEvent = std::make_unique<UpdateEvent>();
            updateEvent->type = Event::Type::Update;
            updateEvent->delta = delta;
            eventDispatcher.dispatchEvent(std::move(updateEvent));
        }

        inputManager->update();
        window->update();
        audio->update();

        if (renderer->getRefillQueue())
            sceneManager.draw();

        if (oneUpdatePerFrame) renderer->waitForNextFrame();
    }

    void Engine::executeOnMainThread(const std::function<void()>& func)
    {
        if (active) runOnMainThread(func);
    }

    void Engine::engineMain()
    {
        Thread::setCurrentThreadName("Application");

        try
        {
            std::unique_ptr<Application> application = ouzel::main(args);

#if !defined(__EMSCRIPTEN__)
            while (active)
            {
                if (!paused)
                    update();
                else
                {
                    std::unique_lock<std::mutex> lock(updateMutex);
                    while (active && paused)
                        updateCondition.wait(lock);
                }
            }

            eventDispatcher.dispatchEvents();
#endif
        }
        catch (const std::exception& e)
        {
            log(Log::Level::Error) << e.what();
            exit();
        }
    }

    void Engine::openUrl(const std::string&)
    {
    }

    void Engine::setScreenSaverEnabled(bool newScreenSaverEnabled)
    {
        screenSaverEnabled = newScreenSaverEnabled;
    }
}
