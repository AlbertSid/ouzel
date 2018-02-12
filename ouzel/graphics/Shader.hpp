// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <vector>
#include "graphics/DataType.hpp"
#include "graphics/Vertex.hpp"

namespace ouzel
{
    namespace graphics
    {
        class ShaderResource;

        class Shader
        {
        public:
            struct ConstantInfo
            {
                ConstantInfo(const std::string& aName, DataType aDataType):
                    name(aName), dataType(aDataType), size(getDataTypeSize(aDataType)) {}

                std::string name;
                DataType dataType;
                uint32_t size;
            };

            Shader();
            virtual ~Shader();

            Shader(const Shader&) = delete;
            Shader& operator=(const Shader&) = delete;

            Shader(Shader&&) = delete;
            Shader& operator=(Shader&&) = delete;

            bool init(const std::vector<uint8_t>& newPixelShader,
                      const std::vector<uint8_t>& newVertexShader,
                      const std::set<Vertex::Attribute::Usage>& newVertexAttributes,
                      const std::vector<ConstantInfo>& newPixelShaderConstantInfo,
                      const std::vector<ConstantInfo>& newVertexShaderConstantInfo,
                      uint32_t newPixelShaderDataAlignment = 0,
                      uint32_t newVertexShaderDataAlignment = 0,
                      const std::string& pixelShaderFunction = "",
                      const std::string& vertexShaderFunction = "");
            bool init(const std::string& newPixelShader,
                      const std::string& newVertexShader,
                      const std::set<Vertex::Attribute::Usage>& newVertexAttributes,
                      const std::vector<ConstantInfo>& newPixelShaderConstantInfo,
                      const std::vector<ConstantInfo>& newVertexShaderConstantInfo,
                      uint32_t newPixelShaderDataAlignment = 0,
                      uint32_t newVertexShaderDataAlignment = 0,
                      const std::string& newPixelShaderFunction = "",
                      const std::string& newVertexShaderFunction = "");

            inline ShaderResource* getResource() const { return resource; }

            const std::set<Vertex::Attribute::Usage>& getVertexAttributes() const;

        private:
            ShaderResource* resource = nullptr;

            std::set<Vertex::Attribute::Usage> vertexAttributes;

            std::string pixelShaderFilename;
            std::string vertexShaderFilename;
        };
    } // namespace graphics
} // namespace ouzel
