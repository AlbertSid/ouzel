// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <string>
#include <mutex>
#include "math/Size2.hpp"

namespace ouzel
{
    class WindowResource
    {
    public:
        class Listener
        {
        public:
            virtual ~Listener() = 0;
            virtual void onSizeChange(const Size2& newSize) = 0;
            virtual void onResolutionChange(const Size2& newResolution) = 0;
            virtual void onFullscreenChange(bool newFullscreen) = 0;
            virtual void onScreenChange(uint32_t newDisplayId) = 0;
            virtual void onClose() = 0;
        };

        WindowResource() = default;
        virtual ~WindowResource() {}

        WindowResource(const WindowResource&) = delete;
        WindowResource& operator=(const WindowResource&) = delete;

        WindowResource(WindowResource&&) = delete;
        WindowResource& operator=(WindowResource&&) = delete;

        virtual bool init(const Size2& newSize,
                          bool newResizable,
                          bool newFullscreen,
                          bool newExclusiveFullscreen,
                          const std::string& newTitle,
                          bool newHighDpi,
                          bool depth);

        virtual void close();

        inline const Size2& getSize() const { return size; }
        virtual void setSize(const Size2& newSize);
        inline const Size2& getResolution() const { return resolution; }
        inline float getContentScale() const { return contentScale; }

        inline bool isResizable() const { return resizable; }

        virtual void setFullscreen(bool newFullscreen);
        inline bool isFullscreen() const { return fullscreen; }

        inline bool isExclusiveFullscreen() const { return exclusiveFullscreen; }

        inline const std::string& getTitle() const { return title; }
        virtual void setTitle(const std::string& newTitle);

        void setListener(Listener* newListener);

    protected:
        Size2 size;
        Size2 resolution;
        float contentScale = 1.0f;
        bool resizable = false;
        bool fullscreen = false;
        bool exclusiveFullscreen = false;
        bool highDpi = true;

        std::string title;

        std::mutex listenerMutex;
        Listener* listener = nullptr;
    };
}

