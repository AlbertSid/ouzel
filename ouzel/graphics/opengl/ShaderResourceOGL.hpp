// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "core/Setup.h"

#if OUZEL_COMPILE_OPENGL

#if OUZEL_SUPPORTS_OPENGLES
    #define GL_GLEXT_PROTOTYPES 1
    #include "GLES/gl.h"
    #include "GLES2/gl2.h"
    #include "GLES2/gl2ext.h"
    #include "GLES3/gl3.h"
#else
    #define GL_GLEXT_PROTOTYPES 1
    #include "GL/glcorearb.h"
    #include "GL/glext.h"
#endif

#include "graphics/ShaderResource.hpp"

namespace ouzel
{
    namespace graphics
    {
        class RenderDeviceOGL;

        class ShaderResourceOGL: public ShaderResource
        {
        public:
            ShaderResourceOGL(RenderDeviceOGL* aRenderDeviceOGL);
            virtual ~ShaderResourceOGL();

            virtual bool init(const std::vector<uint8_t>& newPixelShader,
                              const std::vector<uint8_t>& newVertexShader,
                              const std::vector<Vertex::Attribute>& newVertexAttributes,
                              const std::vector<Shader::ConstantInfo>& newPixelShaderConstantInfo,
                              const std::vector<Shader::ConstantInfo>& newVertexShaderConstantInfo,
                              uint32_t newPixelShaderDataAlignment = 0,
                              uint32_t newVertexShaderDataAlignment = 0,
                              const std::string& newPixelShaderFunction = "",
                              const std::string& newVertexShaderFunction = "") override;

            virtual bool reload() override;

            struct Location
            {
                GLint location;
                DataType dataType;
            };

            const std::vector<Location>& getPixelShaderConstantLocations() const { return pixelShaderConstantLocations; }
            const std::vector<Location>& getVertexShaderConstantLocations() const { return vertexShaderConstantLocations; }

            GLuint getProgramId() const { return programId; }

        protected:
            bool compileShader();
            void printShaderMessage(GLuint shaderId);
            void printProgramMessage();

            RenderDeviceOGL* renderDeviceOGL;

            GLuint pixelShaderId = 0;
            GLuint vertexShaderId = 0;
            GLuint programId = 0;

            std::vector<Location> pixelShaderConstantLocations;
            std::vector<Location> vertexShaderConstantLocations;
        };
    } // namespace graphics
} // namespace ouzel

#endif
