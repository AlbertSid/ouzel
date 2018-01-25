// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "core/Engine.hpp"

namespace ouzel
{
    class EngineEm: public Engine
    {
    public:
        EngineEm(int argc, char* argv[]);

        virtual int run() override;

        bool step();

        virtual void executeOnMainThread(const std::function<void(void)>& func) override;
        virtual bool openURL(const std::string& url) override;
    };
}
