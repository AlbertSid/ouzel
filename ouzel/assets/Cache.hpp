// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <memory>
#include <string>
#include <map>
#include "utils/Noncopyable.hpp"
#include "assets/LoaderBMF.hpp"
#include "assets/LoaderCollada.hpp"
#include "assets/LoaderImage.hpp"
#include "assets/LoaderMTL.hpp"
#include "assets/LoaderOBJ.hpp"
#include "assets/LoaderParticleSystem.hpp"
#include "assets/LoaderSprite.hpp"
#include "assets/LoaderTTF.hpp"
#include "assets/LoaderVorbis.hpp"
#include "assets/LoaderWave.hpp"
#include "audio/SoundData.hpp"
#include "graphics/BlendState.hpp"
#include "graphics/Material.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Texture.hpp"
#include "gui/Font.hpp"
#include "scene/ModelData.hpp"
#include "scene/SpriteData.hpp"
#include "scene/ParticleSystemData.hpp"

namespace ouzel
{
    namespace assets
    {
        class Loader;

        class Cache: public Noncopyable
        {
        public:
            Cache();
            ~Cache();

            void addLoader(Loader* loader);
            void removeLoader(Loader* loader);

            bool loadAsset(const std::string& filename, bool mipmaps = true) const;
            bool loadAsset(uint32_t loaderType, const std::string& filename, bool mipmaps = true) const;
            bool loadAssets(const std::vector<std::string>& filenames, bool mipmaps = true) const;

            void clear();

            const std::shared_ptr<graphics::Texture>& getTexture(const std::string& filename, bool mipmaps = true) const;
            void setTexture(const std::string& filename, const std::shared_ptr<graphics::Texture>& texture);
            void releaseTextures();

            const std::shared_ptr<graphics::Shader>& getShader(const std::string& shaderName) const;
            void setShader(const std::string& shaderName, const std::shared_ptr<graphics::Shader>& shader);
            void releaseShaders();

            const std::shared_ptr<graphics::BlendState>& getBlendState(const std::string& blendStateName) const;
            void setBlendState(const std::string& blendStateName, const std::shared_ptr<graphics::BlendState>& blendState);
            void releaseBlendStates();

            bool preloadSpriteData(const std::string& filename, bool mipmaps = true,
                                         uint32_t spritesX = 1, uint32_t spritesY = 1,
                                         const Vector2& pivot = Vector2(0.5f, 0.5f));
            const scene::SpriteData& getSpriteData(const std::string& filename, bool mipmaps = true,
                                                               uint32_t spritesX = 1, uint32_t spritesY = 1,
                                                               const Vector2& pivot = Vector2(0.5f, 0.5f)) const;
            void setSpriteData(const std::string& filename, const scene::SpriteData& newSpriteData);
            void releaseSpriteData();

            const scene::ParticleSystemData& getParticleSystemData(const std::string& filename, bool mipmaps = true) const;
            void setParticleSystemData(const std::string& filename, const scene::ParticleSystemData& newParticleSystemData);
            void releaseParticleSystemData();

            const std::shared_ptr<Font>& getFont(const std::string& filename, bool mipmaps = true) const;
            void setFont(const std::string& filename, const std::shared_ptr<Font>& font);
            void releaseFonts();

            const std::shared_ptr<audio::SoundData>& getSoundData(const std::string& filename) const;
            void setSoundData(const std::string& filename, const std::shared_ptr<audio::SoundData>& newSoundData);
            void releaseSoundData();

            const std::shared_ptr<graphics::Material>& getMaterial(const std::string& filename, bool mipmaps = true) const;
            void setMaterial(const std::string& filename, const std::shared_ptr<graphics::Material>& material);
            void releaseMaterials();

            const scene::ModelData& getModelData(const std::string& filename, bool mipmaps = true) const;
            void setModelData(const std::string& filename, const scene::ModelData& newModelData);
            void releaseModelData();

        protected:
            LoaderBMF loaderBMF;
            LoaderCollada loaderCollada;
            LoaderImage loaderImage;
            LoaderMTL loaderMTL;
            LoaderOBJ loaderOBJ;
            LoaderParticleSystem loaderParticleSystem;
            LoaderSprite loaderSprite;
            LoaderTTF loaderTTF;
            LoaderVorbis loaderVorbis;
            LoaderWave loaderWave;
            std::vector<Loader*> loaders;
            mutable std::map<std::string, std::shared_ptr<graphics::Texture>> textures;
            mutable std::map<std::string, std::shared_ptr<graphics::Shader>> shaders;
            mutable std::map<std::string, scene::ParticleSystemData> particleSystemData;
            mutable std::map<std::string, std::shared_ptr<graphics::BlendState>> blendStates;
            mutable std::map<std::string, scene::SpriteData> spriteData;
            mutable std::map<std::string, std::shared_ptr<Font>> fonts;
            mutable std::map<std::string, std::shared_ptr<audio::SoundData>> soundData;
            mutable std::map<std::string, std::shared_ptr<graphics::Material>> materials;
            mutable std::map<std::string, scene::ModelData> modelData;
        };
    } // namespace assets
} // namespace ouzel
