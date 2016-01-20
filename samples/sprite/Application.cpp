// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "Application.h"
#include <cmath>

namespace ouzel
{    
    Application::~Application()
    {
        Engine::getInstance()->getEventDispatcher()->removeEventHandler(_eventHandler);
    }
    
    Settings Application::getSettings()
    {
        Settings settings;
        settings.size = ouzel::Size2(800.0f, 600.0f);
        settings.resizable = true;
        
        return settings;
    }
    
    void Application::begin()
    {
        _eventHandler = std::make_shared<EventHandler>();
        
        _eventHandler->keyDownHandler = std::bind(&Application::handleKeyDown, this, std::placeholders::_1, std::placeholders::_2);
        _eventHandler->mouseMoveHandler = std::bind(&Application::handleMouseMove, this, std::placeholders::_1, std::placeholders::_2);
        _eventHandler->touchBeginHandler = std::bind(&Application::handleTouch, this, std::placeholders::_1, std::placeholders::_2);
        _eventHandler->touchMoveHandler = std::bind(&Application::handleTouch, this, std::placeholders::_1, std::placeholders::_2);
        _eventHandler->touchEndHandler = std::bind(&Application::handleTouch, this, std::placeholders::_1, std::placeholders::_2);
        _eventHandler->gamepadButtonChangeHandler = std::bind(&Application::handleGamepadButtonChange, this, std::placeholders::_1, std::placeholders::_2);
        
        Engine::getInstance()->getEventDispatcher()->addEventHandler(_eventHandler);
        
        Engine::getInstance()->getRenderer()->setClearColor(Color(64, 0, 0));
        Engine::getInstance()->getRenderer()->setTitle("Sample");
        
        ScenePtr scene(new Scene());
        Engine::getInstance()->getSceneManager()->setScene(scene);
        
        _layer = std::make_shared<Layer>();
        scene->addLayer(_layer);
        
        _uiLayer = std::make_shared<Layer>();
        scene->addLayer(_uiLayer);
        
        _sprite = std::make_shared<Sprite>();
        _sprite->initFromFile("run.json");
        _sprite->play(true);
        _layer->addChild(_sprite);
        _sprite->setPosition(Vector2(-300.0f, 0.0f));
        _sprite->animate(AnimatorPtr(new Sequence({
            AnimatorPtr(new MoveTo(4.0f, Vector2(300.0f, 0.0f))),
            AnimatorPtr(new FadeTo(2.0f, 0.4f))
        })));
        
        SpritePtr fire(new Sprite());
        fire->initFromFile("fire.json");
        fire->play(true);
        fire->setPosition(Vector2(-100.0f, -100.0f));
        _layer->addChild(fire);
        fire->animate(AnimatorPtr(new FadeTo(5.0f, 0.5f)));
        
        ParticleSystemPtr flame(new ParticleSystem());
        flame->initFromFile("flame.json");
        _layer->addChild(flame);
        
        _witch = std::make_shared<Sprite>();
        _witch->initFromFile("witch.png");
        _witch->setPosition(Vector2(100.0f, 100.0f));
        _witch->setColor(Color(128, 0, 255, 255));
        _layer->addChild(_witch);
        _witch->animate(AnimatorPtr(new Repeat(AnimatorPtr(new RotateTo(1.0f, PI * 2.0f)), 3)));
        
        LabelPtr label = std::make_shared<Label>("font.fnt", "testing fonts");
        _uiLayer->addChild(label);
        label->animate(AnimatorPtr(AnimatorPtr(new Sequence({
            AnimatorPtr(new Animator(1.0f)), // delay
            AnimatorPtr(new Ease(AnimatorPtr(new MoveTo(4.0f, Vector2(0.0f, -240.0f))), Ease::Type::OUT, Ease::Func::ELASTIC))
        }))));
        
        _button.reset(new Button());
        _button->init("button.png", "button.png", "button_down.png", "button_disabled.png", [this](VoidPtr sender) {
            _sprite->setVisible(!_sprite->isVisible());
        });
        _button->setPosition(Vector2(-200.0f, 200.0f));
        _uiLayer->addChild(_button);
        
        Engine::getInstance()->getInput()->startDiscovery();
    }
    
    bool Application::handleKeyDown(const KeyboardEvent& event, VoidPtr const& sender) const
    {
        Vector2 position = _layer->getCamera()->getPosition();
        
        switch (event.key)
        {
            case KeyboardKey::UP:
                position.y += 10.0f;
                break;
            case KeyboardKey::DOWN:
                position.y -= 10.0f;
                break;
            case KeyboardKey::LEFT:
                position.x -= 10.0f;
                break;
            case KeyboardKey::RIGHT:
                position.x += 10.0f;
                break;
            case KeyboardKey::SPACE:
                _witch->setVisible(!_witch->isVisible());
                break;
            case KeyboardKey::RETURN:
                Engine::getInstance()->getRenderer()->resize(Size2(640.0f, 480.0f));
                break;
            case KeyboardKey::TAB:
                _button->setEnabled(!_button->isEnabled());
                break;
            default:
                break;
        }
        
        _layer->getCamera()->setPosition(position);
        
        return true;
    }
    
    bool Application::handleMouseMove(const MouseEvent& event, VoidPtr const& sender) const
    {
        Vector2 worldLocation = _layer->screenToWorldLocation(event.position);
        _witch->setPosition(worldLocation);
        
        return true;
    }
    
    bool Application::handleTouch(const TouchEvent& event, VoidPtr const& sender) const
    {
        Vector2 worldLocation = _layer->screenToWorldLocation(event.position);
        _witch->setPosition(worldLocation);
        
        return true;
    }
    
    bool Application::handleGamepadButtonChange(const GamepadEvent& event, VoidPtr const& sender) const
    {
        Vector2 position = _layer->worldToScreenLocation(_witch->getPosition());
        
        switch (event.button)
        {
            case GamepadButton::DPAD_UP:
            case GamepadButton::LEFT_THUMB_UP:
            case GamepadButton::RIGHT_THUMB_UP:
                position.y = event.value;
                break;
            case GamepadButton::DPAD_DOWN:
            case GamepadButton::LEFT_THUMB_DOWN:
            case GamepadButton::RIGHT_THUMB_DOWN:
                position.y = -event.value;
                break;
            case GamepadButton::DPAD_LEFT:
            case GamepadButton::LEFT_THUMB_LEFT:
            case GamepadButton::RIGHT_THUMB_LEFT:
                position.x = -event.value;
                break;
            case GamepadButton::DPAD_RIGHT:
            case GamepadButton::LEFT_THUMB_RIGHT:
            case GamepadButton::RIGHT_THUMB_RIGHT:
                position.x = event.value;
                break;
            case GamepadButton::A:
                _witch->setVisible(!event.pressed);
                break;
            default:
                break;
        }
        
        Vector2 worldLocation = _layer->screenToWorldLocation(position);
        _witch->setPosition(worldLocation);
        
        return true;
    }
}
