// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#if defined(__OBJC__)
#include <GameController/GameController.h>
typedef GCController* GCControllerPtr;
#else
#include <objc/objc.h>
typedef id GCControllerPtr;
#endif

#include "input/Gamepad.hpp"

namespace ouzel
{
    namespace input
    {
        class InputMacOS;

        class GamepadGC: public Gamepad
        {
            friend InputMacOS;
        public:
            virtual void setAbsoluteDpadValues(bool absoluteDpadValues) override;
            virtual bool isAbsoluteDpadValues() const override;

            virtual int32_t getPlayerIndex() const override;
            virtual bool setPlayerIndex(int32_t playerIndex) override;

            GCControllerPtr getController() const { return controller; }

        protected:
            GamepadGC(GCControllerPtr aController);

            GCControllerPtr controller;
        };
    } // namespace input
} // namespace ouzel
