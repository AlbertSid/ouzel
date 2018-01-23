// Copyright (C) 2018 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

class RTSample: public ouzel::scene::Scene
{
public:
    RTSample();

private:
    bool handleGamepad(ouzel::Event::Type type, const ouzel::GamepadEvent& event);
    bool handleUI(ouzel::Event::Type type, const ouzel::UIEvent& event) const;
    bool handleKeyboard(ouzel::Event::Type type, const ouzel::KeyboardEvent& event) const;

    ouzel::scene::Layer layer;
    ouzel::scene::Camera camera;
    ouzel::scene::Actor cameraActor;

    ouzel::scene::Layer rtLayer;

    ouzel::scene::Camera rtCamera;
    ouzel::scene::Actor rtCameraActor;
    ouzel::scene::Camera camera1;
    ouzel::scene::Actor camera1Actor;
    ouzel::scene::Camera camera2;
    ouzel::scene::Actor camera2Actor;

    ouzel::scene::Sprite characterSprite;
    ouzel::scene::Actor rtCharacter;

    ouzel::scene::Sprite rtSprite;
    ouzel::scene::Actor rtActor;

    ouzel::EventHandler eventHandler;

    ouzel::scene::Layer guiLayer;
    ouzel::scene::Camera guiCamera;
    ouzel::scene::Actor guiCameraActor;
    ouzel::gui::Menu menu;
    ouzel::gui::Button backButton;
};
