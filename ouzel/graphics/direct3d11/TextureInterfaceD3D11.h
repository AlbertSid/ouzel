// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "core/CompileConfig.h"

#if OUZEL_SUPPORTS_DIRECT3D11

#include <d3d11.h>
#include "graphics/TextureInterface.h"

namespace ouzel
{
    namespace graphics
    {
        class TextureInterfaceD3D11: public TextureInterface
        {
        public:
            TextureInterfaceD3D11();
            virtual ~TextureInterfaceD3D11();

            virtual bool init(const Size2& newSize,
                              uint32_t newFlags = 0,
                              uint32_t newMipmaps = 0,
                              uint32_t newSampleCount = 1,
                              PixelFormat newPixelFormat = PixelFormat::RGBA8_UNORM) override;
            virtual bool init(const std::vector<uint8_t>& newData,
                              const Size2& newSize,
                              uint32_t newFlags = 0,
                              uint32_t newMipmaps = 0,
                              PixelFormat newPixelFormat = PixelFormat::RGBA8_UNORM) override;

            virtual bool setSize(const Size2& newSize) override;
            virtual bool setData(const std::vector<uint8_t>& newData, const Size2& newSize) override;
            virtual bool setFilter(Texture::Filter newFilter) override;
            virtual bool setAddressX(Texture::Address newAddressX) override;
            virtual bool setAddressY(Texture::Address newAddressY) override;
            virtual bool setMaxAnisotropy(uint32_t newMaxAnisotropy) override;
            virtual bool setClearColorBuffer(bool clear) override;
            virtual bool setClearDepthBuffer(bool clear) override;
            virtual bool setClearColor(Color color) override;

            ID3D11Texture2D* getTexture() const { return texture; }
            ID3D11ShaderResourceView* getResourceView() const { return resourceView; }
            ID3D11SamplerState* getSamplerState() const { return samplerState; }

            ID3D11RenderTargetView* getRenderTargetView() const { return renderTargetView; }
            ID3D11Texture2D* getDepthStencilTexture() const { return depthStencilTexture; }
            ID3D11DepthStencilView* getDepthStencilView() const { return depthStencilView; }

            const float* getFrameBufferClearColor() const { return frameBufferClearColor; }

            UINT getWidth() const { return width; }
            UINT getHeight() const { return height; }

            bool getClearFrameBufferView() const { return clearFrameBufferView; }
            bool getClearDepthBufferView() const { return clearDepthBufferView; }

        protected:
            bool updateTexture();
            bool updateSamplerState();

            ID3D11Texture2D* texture = nullptr;
            ID3D11ShaderResourceView* resourceView = nullptr;
            ID3D11SamplerState* samplerState = nullptr;

            UINT width = 0;
            UINT height = 0;

            ID3D11RenderTargetView* renderTargetView = nullptr;
            ID3D11Texture2D* depthStencilTexture = nullptr;
            ID3D11DepthStencilView* depthStencilView = nullptr;

            FLOAT frameBufferClearColor[4];
            bool clearFrameBufferView = true;
            bool clearDepthBufferView = false;
        };
    } // namespace graphics
} // namespace ouzel

#endif
