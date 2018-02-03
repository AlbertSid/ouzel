// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <memory>
#include <vector>

namespace ouzel
{
    class Engine;

    namespace scene
    {
        class Scene;

        class SceneManager
        {
            friend Engine;
        public:
            virtual ~SceneManager();

            SceneManager(const SceneManager&) = delete;
            SceneManager& operator=(const SceneManager&) = delete;

            SceneManager(const SceneManager&&) = delete;
            SceneManager& operator=(const SceneManager&&) = delete;

            void draw();

            void setScene(Scene* scene)
            {
                addChildScene(scene);
            }
            template<class T> void setScene(const std::unique_ptr<T>& scene)
            {
                addChildScene(scene.get());
            }
            template<class T> void setScene(std::unique_ptr<T>&& scene)
            {
                addChildScene(scene.get());
                ownedScenes.push_back(std::move(scene));
            }

            bool removeScene(Scene* scene)
            {
                return removeChildScene(scene);
            }
            template<class T> bool setScene(const std::unique_ptr<T>& scene)
            {
                return removeChildScene(scene.get());
            }
            inline Scene* getScene() const { return scenes.empty() ? nullptr : scenes.back(); }

        protected:
            SceneManager();
            virtual void addChildScene(Scene* scene);
            virtual bool removeChildScene(Scene* scene);

            std::vector<Scene*> scenes;
            std::vector<std::unique_ptr<Scene>> ownedScenes;
        };
    } // namespace scene
} // namespace ouzel
