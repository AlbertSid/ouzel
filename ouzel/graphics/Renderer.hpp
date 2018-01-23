// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <queue>
#include <set>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "utils/Noncopyable.hpp"
#include "math/Rectangle.hpp"
#include "math/Matrix4.hpp"
#include "math/Size2.hpp"
#include "math/Color.hpp"
#include "graphics/Texture.hpp"

namespace ouzel
{
    class Engine;
    class Window;

    namespace graphics
    {
        const std::string SHADER_TEXTURE = "shaderTexture";
        const std::string SHADER_COLOR = "shaderColor";

        const std::string BLEND_NO_BLEND = "blendNoBlend";
        const std::string BLEND_ADD = "blendAdd";
        const std::string BLEND_MULTIPLY = "blendMultiply";
        const std::string BLEND_ALPHA = "blendAlpha";
        const std::string BLEND_SCREEN = "blendScreen";

        const std::string TEXTURE_WHITE_PIXEL = "textureWhitePixel";

        class RenderDevice;
        class BlendState;
        class MeshBuffer;
        class Shader;

        class Renderer: public Noncopyable
        {
            friend Engine;
            friend Window;
        public:
            enum class Driver
            {
                DEFAULT,
                EMPTY,
                OPENGL,
                DIRECT3D11,
                METAL
            };

            enum class DrawMode
            {
                POINT_LIST,
                LINE_LIST,
                LINE_STRIP,
                TRIANGLE_LIST,
                TRIANGLE_STRIP
            };

            enum class CullMode
            {
                NONE,
                FRONT,
                BACK
            };

            ~Renderer();

            static std::set<Driver> getAvailableRenderDrivers();

            inline RenderDevice* getDevice() const { return device.get(); }

            void executeOnRenderThread(const std::function<void(void)>& func);

            void setClearColorBuffer(bool clear);
            bool getClearColorBuffer() const { return clearColorBuffer; }

            void setClearDepthBuffer(bool clear);
            bool getClearDepthBuffer() const { return clearDepthBuffer; }

            void setClearColor(Color color);
            Color getClearColor() const { return clearColor; }

            void setClearDepth(float newClearDepth);
            float getClearDepth() const { return clearDepth; }

            const Size2& getSize() const { return size; }

            bool saveScreenshot(const std::string& filename);

            bool addDrawCommand(const std::vector<std::shared_ptr<Texture>>& textures,
                                const std::shared_ptr<Shader>& shader,
                                const std::vector<std::vector<float>>& pixelShaderConstants,
                                const std::vector<std::vector<float>>& vertexShaderConstants,
                                const std::shared_ptr<BlendState>& blendState,
                                const std::shared_ptr<MeshBuffer>& meshBuffer,
                                uint32_t indexCount,
                                DrawMode drawMode,
                                uint32_t startIndex,
                                const std::shared_ptr<Texture>& renderTarget,
                                const Rectangle& viewport,
                                bool depthWrite,
                                bool depthTest,
                                bool wireframe,
                                bool scissorTest,
                                const Rectangle& scissorRectangle,
                                CullMode cullMode);

        protected:
            Renderer(Driver aDriver);
            bool init(Window* newWindow,
                      const Size2& newSize,
                      uint32_t newSampleCount,
                      Texture::Filter newTextureFilter,
                      uint32_t newMaxAnisotropy,
                      bool newVerticalSync,
                      bool newDepth,
                      bool newDebugRenderer);

            void setSize(const Size2& newSize);

            std::unique_ptr<RenderDevice> device;

            Size2 size;
            Color clearColor;
            float clearDepth = 1.0;
            bool clearColorBuffer = true;
            bool clearDepthBuffer = false;
        };
    } // namespace graphics
} // namespace ouzel
