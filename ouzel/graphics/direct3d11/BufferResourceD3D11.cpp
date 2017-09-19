// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "core/Setup.h"

#if OUZEL_COMPILE_DIRECT3D11

#include "BufferResourceD3D11.hpp"
#include "RenderDeviceD3D11.hpp"
#include "utils/Log.hpp"

namespace ouzel
{
    namespace graphics
    {
        BufferResourceD3D11::BufferResourceD3D11(RenderDeviceD3D11* aRenderDeviceD3D11):
            renderDeviceD3D11(aRenderDeviceD3D11)
        {
        }

        BufferResourceD3D11::~BufferResourceD3D11()
        {
            if (buffer)
            {
                buffer->Release();
            }
        }

        bool BufferResourceD3D11::init(Buffer::Usage newUsage, uint32_t newFlags, uint32_t newSize)
        {
            if (!BufferResource::init(newUsage, newFlags, newSize))
            {
                return false;
            }

            if (!createBuffer())
            {
                return false;
            }

            return true;
        }

        bool BufferResourceD3D11::init(Buffer::Usage newUsage, const std::vector<uint8_t>& newData, uint32_t newFlags)
        {
            if (!BufferResource::init(newUsage, newData, newFlags))
            {
                return false;
            }

            if (!createBuffer())
            {
                return false;
            }

            return true;
        }

        bool BufferResourceD3D11::setData(const std::vector<uint8_t>& newData)
        {
            if (!BufferResource::setData(newData))
            {
                return false;
            }

            if (!data.empty())
            {
                if (!buffer || data.size() > bufferSize)
                {
                    createBuffer();
                }
                else
                {
                    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
                    mappedSubresource.pData = nullptr;
                    mappedSubresource.RowPitch = 0;
                    mappedSubresource.DepthPitch = 0;

                    HRESULT hr = renderDeviceD3D11->getContext()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
                    if (FAILED(hr))
                    {
                        Log(Log::Level::ERR) << "Failed to lock Direct3D 11 buffer, error: " << hr;
                        return false;
                    }

                    std::copy(data.begin(), data.end(), static_cast<uint8_t*>(mappedSubresource.pData));

                    renderDeviceD3D11->getContext()->Unmap(buffer, 0);
                }
            }

            return true;
        }

        bool BufferResourceD3D11::createBuffer()
        {
            if (buffer)
            {
                buffer->Release();
                buffer = nullptr;
            }

            bufferSize = static_cast<UINT>(data.size());

            if (!data.empty())
            {
                D3D11_BUFFER_DESC bufferDesc;
                bufferDesc.ByteWidth = bufferSize;
                bufferDesc.Usage = (flags & Texture::DYNAMIC) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;

                switch (usage)
                {
                    case Buffer::Usage::INDEX:
                        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                        break;
                    case Buffer::Usage::VERTEX:
                        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                        break;
                    default:
                        Log(Log::Level::ERR) << "Unsupported buffer type";
                        return false;
                }

                bufferDesc.CPUAccessFlags = (flags & Texture::DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
                bufferDesc.MiscFlags = 0;
                bufferDesc.StructureByteStride = 0;

                D3D11_SUBRESOURCE_DATA bufferResourceData;
                bufferResourceData.pSysMem = data.data();
                bufferResourceData.SysMemPitch = 0;
                bufferResourceData.SysMemSlicePitch = 0;

                HRESULT hr = renderDeviceD3D11->getDevice()->CreateBuffer(&bufferDesc, &bufferResourceData, &buffer);
                if (FAILED(hr))
                {
                    Log(Log::Level::ERR) << "Failed to create Direct3D 11 buffer, error: " << hr;
                    return false;
                }
            }

            return true;
        }
    } // namespace graphics
} // namespace ouzel

#endif
