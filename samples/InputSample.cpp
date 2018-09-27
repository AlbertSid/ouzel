// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "InputSample.hpp"
#include "MainMenu.hpp"

using namespace std;
using namespace ouzel;

class Mover: public scene::Component
{
public:
    Mover():
        scene::Component(10)
    {
        handler.keyboardHandler = bind(&Mover::handleKeyboard, this, placeholders::_1, placeholders::_2);

        engine->getEventDispatcher().addEventHandler(&handler);
    }

    bool handleKeyboard(Event::Type eventType, const KeyboardEvent& event)
    {
        if (actor)
        {
            if (eventType == Event::Type::KEY_PRESS)
            {
                Vector2 position = actor->getPosition();

                switch (event.key)
                {
                    case input::Keyboard::Key::W:
                        position.y += 10.0F;
                        break;
                    case input::Keyboard::Key::S:
                        position.y -= 10.0F;
                        break;
                    case input::Keyboard::Key::A:
                        position.x -= 10.0F;
                        break;
                    case input::Keyboard::Key::D:
                        position.x += 10.0F;
                        break;
                    default:
                        break;
                }

                actor->setPosition(position);
            }
        }

        return false;
    }

    ouzel::EventHandler handler;
};

InputSample::InputSample():
    hideButton("button.png", "button_selected.png", "button_down.png", "", "Show/hide", "arial.fnt", 1.0F, Color::BLACK, Color::BLACK, Color::BLACK),
    discoverButton("button.png", "button_selected.png", "button_down.png", "", "Discover gamepads", "arial.fnt", 0.8F, Color::BLACK, Color::BLACK, Color::BLACK),
    backButton("button.png", "button_selected.png", "button_down.png", "", "Back", "arial.fnt", 1.0F, Color::BLACK, Color::BLACK, Color::BLACK)
{
    cursor.init("cursor.png", Vector2(0.0F, 63.0F));
    engine->getInputManager()->setCursor(&cursor);

    handler.keyboardHandler = bind(&InputSample::handleKeyboard, this, placeholders::_1, placeholders::_2);
    handler.mouseHandler = bind(&InputSample::handleMouse, this, placeholders::_1, placeholders::_2);
    handler.touchHandler = bind(&InputSample::handleTouch, this, placeholders::_1, placeholders::_2);
    handler.gamepadHandler = bind(&InputSample::handleGamepad, this, placeholders::_1, placeholders::_2);
    handler.uiHandler = bind(&InputSample::handleUI, this, placeholders::_1, placeholders::_2);

    engine->getEventDispatcher().addEventHandler(&handler);

    camera.setScaleMode(scene::Camera::ScaleMode::SHOW_ALL);
    camera.setTargetContentSize(Size2(800.0F, 600.0F));
    cameraActor.addComponent(&camera);

    std::unique_ptr<Mover> mover(new Mover());
    cameraActor.addComponent(std::move(mover));

    layer.addChild(&cameraActor);
    addLayer(&layer);

    flameParticleSystem.init("flame.json");

    flame.addComponent(&flameParticleSystem);
    flame.setPickable(false);
    layer.addChild(&flame);

    guiCamera.setScaleMode(scene::Camera::ScaleMode::SHOW_ALL);
    guiCamera.setTargetContentSize(Size2(800.0F, 600.0F));
    guiCameraActor.addComponent(&guiCamera);
    guiLayer.addChild(&guiCameraActor);
    addLayer(&guiLayer);

    guiLayer.addChild(&menu);

    hideButton.setPosition(Vector2(-200.0F, 200.0F));
    menu.addWidget(&hideButton);

    discoverButton.setPosition(Vector2(-200.0F, 140.0F));
    menu.addWidget(&discoverButton);

    backButton.setPosition(Vector2(-200.0F, -200.0F));
    menu.addWidget(&backButton);
}

bool InputSample::handleKeyboard(Event::Type type, const KeyboardEvent& event)
{
    if (type == Event::Type::KEY_PRESS)
    {
        Vector2 flamePosition = camera.convertWorldToNormalized(flame.getPosition());

        switch (event.key)
        {
            case input::Keyboard::Key::UP:
                flamePosition.y += 0.01F;
                break;
            case input::Keyboard::Key::DOWN:
                flamePosition.y -= 0.01F;
                break;
            case input::Keyboard::Key::LEFT:
                flamePosition.x -= 0.01F;
                break;
            case input::Keyboard::Key::RIGHT:
                flamePosition.x += 0.01F;
                break;
            case input::Keyboard::Key::R:
                engine->getWindow()->setSize(Size2(640.0F, 480.0F));
                break;
            case input::Keyboard::Key::TAB:
                hideButton.setEnabled(!hideButton.isEnabled());
                break;
            case input::Keyboard::Key::ESCAPE:
            case input::Keyboard::Key::MENU:
                engine->getInputManager()->setCursorVisible(true);
                engine->getSceneManager().setScene(std::unique_ptr<scene::Scene>(new MainMenu()));
                return false;
            default:
                break;
        }

        Vector2 worldLocation = camera.convertNormalizedToWorld(flamePosition);

        flame.setPosition(worldLocation);
    }

    return false;
}

bool InputSample::handleMouse(Event::Type type, const MouseEvent& event)
{
    switch (type)
    {
        case Event::Type::MOUSE_MOVE:
        {
            Vector2 worldLocation = camera.convertNormalizedToWorld(event.position);
            flame.setPosition(worldLocation);
            break;
        }
        default:
            break;
    }

    return false;
}

bool InputSample::handleTouch(Event::Type, const TouchEvent& event)
{
    Vector2 worldLocation = camera.convertNormalizedToWorld(event.position);
    flame.setPosition(worldLocation);

    return false;
}

bool InputSample::handleGamepad(Event::Type type, const GamepadEvent& event)
{
    if (type == Event::Type::GAMEPAD_BUTTON_CHANGE)
    {
        Vector2 flamePosition = camera.convertWorldToNormalized(flame.getPosition());

        switch (event.button)
        {
            case input::Gamepad::Button::FACE_3:
                if (event.pressed) engine->getSceneManager().setScene(std::unique_ptr<scene::Scene>(new MainMenu()));
                return false;
            case input::Gamepad::Button::DPAD_UP:
            case input::Gamepad::Button::LEFT_THUMB_UP:
            case input::Gamepad::Button::RIGHT_THUMB_UP:
                flamePosition.y = event.value / 2.0F + 0.5F;
                break;
            case input::Gamepad::Button::DPAD_DOWN:
            case input::Gamepad::Button::LEFT_THUMB_DOWN:
            case input::Gamepad::Button::RIGHT_THUMB_DOWN:
                flamePosition.y = -event.value / 2.0F + 0.5F;
                break;
            case input::Gamepad::Button::DPAD_LEFT:
            case input::Gamepad::Button::LEFT_THUMB_LEFT:
            case input::Gamepad::Button::RIGHT_THUMB_LEFT:
                flamePosition.x = -event.value / 2.0F + 0.5F;
                break;
            case input::Gamepad::Button::DPAD_RIGHT:
            case input::Gamepad::Button::LEFT_THUMB_RIGHT:
            case input::Gamepad::Button::RIGHT_THUMB_RIGHT:
                flamePosition.x = event.value / 2.0F + 0.5F;
                break;
            default:
                break;
        }

        Vector2 worldLocation = camera.convertNormalizedToWorld(flamePosition);
        flame.setPosition(worldLocation);
    }

    return false;
}

bool InputSample::handleUI(Event::Type type, const UIEvent& event) const
{
    if (type == Event::Type::ACTOR_CLICK)
    {
        if (event.actor == &backButton)
        {
            engine->getInputManager()->setCursorVisible(true);
            engine->getSceneManager().setScene(std::unique_ptr<scene::Scene>(new MainMenu()));
        }
        else if (event.actor == &hideButton)
            engine->getInputManager()->setCursorVisible(!engine->getInputManager()->isCursorVisible());
        else if (event.actor == &discoverButton)
            engine->getInputManager()->startDeviceDiscovery();
    }

    return false;
}
