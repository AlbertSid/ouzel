// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "Gamepad.h"
#include "Engine.h"

namespace ouzel
{
    bool Gamepad::isAttached() const
    {
        return false;
    }
    
    void Gamepad::setAbsoluteDpadValues(bool absoluteDpadValues)
    {
        
    }
    
    bool Gamepad::isAbsoluteDpadValues() const
    {
        return false;
    }
    
    int32_t Gamepad::getPlayerIndex() const
    {
        return -1;
    }
    
    bool Gamepad::setPlayerIndex(int32_t playerIndex)
    {
        return false;
    }
    
    void Gamepad::handleButtonValueChange(GamepadButton button, bool pressed, float value)
    {
        GamepadEvent event;
        event.type = Event::Type::GAMEPAD_BUTTON_CHANGE;
        event.gamepad = shared_from_this();
        event.button = button;
        event.pressed = pressed;
        event.value = value;
        
        Engine::getInstance()->getEventDispatcher()->dispatchGamepadButtonChangeEvent(event, Engine::getInstance()->getInput());
    }
}
