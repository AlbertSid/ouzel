// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <algorithm>
#import <GameController/GameController.h>
#include "InputTVOS.h"
#include "GamepadTVOS.h"
#include "core/tvos/WindowTVOS.h"
#include "core/Engine.h"
#include "events/EventDispatcher.h"
#include "utils/Log.h"

@interface ConnectDelegate: NSObject
{
    ouzel::input::InputTVOS* input;
}

-(void)handleControllerConnected:(NSNotification*)notification;
-(void)handleControllerDisconnected:(NSNotification*)notification;

@end

@implementation ConnectDelegate

-(id)initWithInput:(ouzel::input::InputTVOS*)newInput
{
    if (self = [super init])
    {
        input = newInput;
    }

    return self;
}

-(void)handleControllerConnected:(NSNotification*)notification
{
    input->handleGamepadConnected(notification.object);
}

-(void)handleControllerDisconnected:(NSNotification*)notification
{
    input->handleGamepadDisconnected(notification.object);
}

@end

namespace ouzel
{
    namespace input
    {
        InputTVOS::InputTVOS()
        {
        }

        InputTVOS::~InputTVOS()
        {
            if (connectDelegate)
            {
                [[NSNotificationCenter defaultCenter] removeObserver:connectDelegate];

                [connectDelegate release];
            }
        }

        bool InputTVOS::init()
        {
            connectDelegate = [[ConnectDelegate alloc] initWithInput:this];

            [[NSNotificationCenter defaultCenter] addObserver:connectDelegate
                                                     selector:@selector(handleControllerConnected:)
                                                         name:GCControllerDidConnectNotification
                                                       object:Nil];

            [[NSNotificationCenter defaultCenter] addObserver:connectDelegate
                                                     selector:@selector(handleControllerDisconnected:)
                                                         name:GCControllerDidDisconnectNotification
                                                       object:Nil];


            for (GCController* controller in [GCController controllers])
            {
                handleGamepadConnected(controller);
            }

            startGamepadDiscovery();
            
            return true;
        }

        void InputTVOS::startGamepadDiscovery()
        {
            Log(Log::Level::INFO) << "Started gamepad discovery";

            discovering = true;

            [GCController startWirelessControllerDiscoveryWithCompletionHandler:
             ^(void){ handleGamepadDiscoveryCompleted(); }];
        }

        void InputTVOS::stopGamepadDiscovery()
        {
            if (discovering)
            {
                Log(Log::Level::INFO) << "Stopped gamepad discovery";

                [GCController stopWirelessControllerDiscovery];

                discovering = false;
            }
        }

        bool InputTVOS::showVirtualKeyboard()
        {
            sharedEngine->executeOnMainThread([]() {
                WindowTVOS* windowTVOS = static_cast<WindowTVOS*>(sharedEngine->getWindow());
                UITextField* textField = windowTVOS->getTextField();
                [textField becomeFirstResponder];
            });

            return true;
        }

        bool InputTVOS::hideVirtualKeyboard()
        {
            sharedEngine->executeOnMainThread([]() {
                WindowTVOS* windowTVOS = static_cast<WindowTVOS*>(sharedEngine->getWindow());
                UITextField* textField = windowTVOS->getTextField();
                [textField resignFirstResponder];
            });

            return true;
        }

        void InputTVOS::handleGamepadDiscoveryCompleted()
        {
            Log(Log::Level::INFO) << "Gamepad discovery completed";
            discovering = false;
        }

        void InputTVOS::handleGamepadConnected(GCControllerPtr controller)
        {
            std::vector<int32_t> playerIndices = {0, 1, 2, 3};

            for (const auto& gamepad : gamepads)
            {
                auto i = std::find(playerIndices.begin(), playerIndices.end(), gamepad->getPlayerIndex());

                if (i != playerIndices.end()) playerIndices.erase(i);
            }

            if (!playerIndices.empty()) controller.playerIndex = static_cast<GCControllerPlayerIndex>(playerIndices.front());

            Event event;
            event.type = Event::Type::GAMEPAD_CONNECT;

            std::unique_ptr<GamepadTVOS> gamepad(new GamepadTVOS(controller));

            event.gamepadEvent.gamepad = gamepad.get();

            gamepads.push_back(std::move(gamepad));

            sharedEngine->getEventDispatcher()->postEvent(event);
        }

        void InputTVOS::handleGamepadDisconnected(GCControllerPtr controller)
        {
            std::vector<std::unique_ptr<Gamepad>>::iterator i = std::find_if(gamepads.begin(), gamepads.end(), [controller](const std::unique_ptr<Gamepad>& gamepad) {
                GamepadTVOS* currentGamepad = static_cast<GamepadTVOS*>(gamepad.get());
                return currentGamepad->getController() == controller;
            });

            if (i != gamepads.end())
            {
                Event event;
                event.type = Event::Type::GAMEPAD_DISCONNECT;

                event.gamepadEvent.gamepad = (*i).get();

                sharedEngine->getEventDispatcher()->postEvent(event);

                gamepads.erase(i);
            }
        }
    } // namespace input
} // namespace ouzel
