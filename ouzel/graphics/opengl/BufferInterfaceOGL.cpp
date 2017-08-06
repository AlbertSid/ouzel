// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "core/CompileConfig.hpp"

#if OUZEL_SUPPORTS_OPENGL

#include "BufferInterfaceOGL.hpp"
#include "RendererOGL.hpp"
#include "utils/Log.hpp"

namespace ouzel
{
    namespace graphics
    {
        BufferInterfaceOGL::BufferInterfaceOGL(RendererOGL* aRendererOGL):
            rendererOGL(aRendererOGL)
        {
        }

        BufferInterfaceOGL::~BufferInterfaceOGL()
        {
            if (bufferId)
            {
                rendererOGL->deleteBuffer(bufferId);
            }
        }

        bool BufferInterfaceOGL::init(Buffer::Usage newUsage, uint32_t newFlags, uint32_t newSize)
        {
            if (!BufferInterface::init(newUsage, newFlags, newSize))
            {
                return false;
            }

            if (!createBuffer())
            {
                return false;
            }

            bufferSize = static_cast<GLsizeiptr>(data.size());

            if (!data.empty())
            {
                rendererOGL->bindVertexArray(0);

                if (!rendererOGL->bindBuffer(bufferType, bufferId))
                {
                    return false;
                }

                glBufferDataProc(bufferType, bufferSize, nullptr,
                                 (flags & Texture::DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

                if (RendererOGL::checkOpenGLError())
                {
                    Log(Log::Level::ERR) << "Failed to create buffer";
                    return false;
                }
            }

            return true;
        }

        bool BufferInterfaceOGL::init(Buffer::Usage newUsage, const std::vector<uint8_t>& newData, uint32_t newFlags)
        {
            if (!BufferInterface::init(newUsage, newData, newFlags))
            {
                return false;
            }

            if (!createBuffer())
            {
                return false;
            }

            bufferSize = static_cast<GLsizeiptr>(data.size());

            if (!data.empty())
            {
                rendererOGL->bindVertexArray(0);

                if (!rendererOGL->bindBuffer(bufferType, bufferId))
                {
                    return false;
                }

                glBufferDataProc(bufferType, bufferSize, data.data(),
                                 (flags & Texture::DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

                if (RendererOGL::checkOpenGLError())
                {
                    Log(Log::Level::ERR) << "Failed to create buffer";
                    return false;
                }
            }

            return true;
        }

        bool BufferInterfaceOGL::setData(const std::vector<uint8_t>& newData)
        {
            if (!BufferInterface::setData(newData))
            {
                return false;
            }

            if (!bufferId)
            {
                Log(Log::Level::ERR) << "Buffer not initialized";
                return false;
            }

            if (!data.empty())
            {
                rendererOGL->bindVertexArray(0);

                if (!rendererOGL->bindBuffer(bufferType, bufferId))
                {
                    return false;
                }

                if (static_cast<GLsizeiptr>(data.size()) > bufferSize)
                {
                    bufferSize = static_cast<GLsizeiptr>(data.size());

                    glBufferDataProc(bufferType, bufferSize, data.data(),
                                     (flags & Texture::DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

                    if (RendererOGL::checkOpenGLError())
                    {
                        Log(Log::Level::ERR) << "Failed to create buffer";
                        return false;
                    }
                }
                else
                {
                    glBufferSubDataProc(bufferType, 0, static_cast<GLsizeiptr>(data.size()), data.data());

                    if (RendererOGL::checkOpenGLError())
                    {
                        Log(Log::Level::ERR) << "Failed to upload buffer";
                        return false;
                    }
                }
            }

            return true;
        }

        bool BufferInterfaceOGL::createBuffer()
        {
            if (bufferId) rendererOGL->deleteBuffer(bufferId);

            glGenBuffersProc(1, &bufferId);

            if (RendererOGL::checkOpenGLError())
            {
                Log(Log::Level::ERR) << "Failed to create buffer";
                return false;
            }

            switch (usage)
            {
                case Buffer::Usage::INDEX:
                    bufferType = GL_ELEMENT_ARRAY_BUFFER;
                    break;
                case Buffer::Usage::VERTEX:
                    bufferType = GL_ARRAY_BUFFER;
                    break;
                default:
                    bufferType = 0;
                    Log(Log::Level::ERR) << "Unsupported buffer type";
                    return false;
            }

            return true;
        }
    } // namespace graphics
} // namespace ouzel

#endif
