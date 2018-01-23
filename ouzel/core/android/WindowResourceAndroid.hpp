// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "core/WindowResource.hpp"

namespace ouzel
{
    class Window;

    class WindowResourceAndroid: public WindowResource
    {
        friend Window;
    public:
        virtual ~WindowResourceAndroid();

        void handleResize(const Size2& newSize);
        void handleSurfaceChange(jobject surface);
        void handleSurfaceDestroy();

        ANativeWindow* getNativeWindow() const { return window; }

    protected:
        WindowResourceAndroid();
        virtual bool init(const Size2& newSize,
                          bool newResizable,
                          bool newFullscreen,
                          bool newExclusiveFullscreen,
                          const std::string& newTitle,
                          bool newHighDpi,
                          bool depth);

        ANativeWindow* window = nullptr;
    };
}
