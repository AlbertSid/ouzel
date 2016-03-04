// Copyright (C) 2016 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <algorithm>
#include "Engine.h"
#include "CompileConfig.h"
#include "Cache.h"

#if defined(OUZEL_PLATFORM_OSX)
#include "WindowOSX.h"
#elif defined(OUZEL_PLATFORM_IOS)
#include "WindowIOS.h"
#elif defined(OUZEL_PLATFORM_TVOS)
#include "WindowTVOS.h"
#elif defined(OUZEL_PLATFORM_ANDROID)
#include "WindowAndroid.h"
#elif defined(OUZEL_PLATFORM_LINUX)
#include "WindowLinux.h"
#elif defined(OUZEL_PLATFORM_WINDOWS)
#include "WindowWin.h"
#endif

#if defined(OUZEL_SUPPORTS_OPENGL) || defined(OUZEL_SUPPORTS_OPENGLES)
#include "RendererOGL.h"
#endif

#if defined(SUPPORTS_DIRECT3D11)
#include "RendererD3D11.h"
#endif

#include "Utils.h"
#include "Renderer.h"
#include "FileSystem.h"

#if defined(OUZEL_PLATFORM_OSX) || defined(OUZEL_PLATFORM_IOS) || defined(OUZEL_PLATFORM_TVOS)
#include "InputApple.h"
#elif defined(OUZEL_PLATFORM_WINDOWS)
#include "InputWin.h"
#endif

ouzel::AppPtr ouzelMain(const std::vector<std::string>& args);

namespace ouzel
{
    static EnginePtr sharedEngine;

    EnginePtr Engine::getInstance()
    {
        if (!sharedEngine)
        {
            sharedEngine.reset(new Engine());
        }

        return sharedEngine;
    }

    Engine::Engine()
    {

    }

    Engine::~Engine()
    {

    }

    void Engine::setArgs(const std::vector<std::string>& args)
    {
        _args = args;
    }

    std::set<Renderer::Driver> Engine::getAvailableDrivers() const
    {
        std::set<Renderer::Driver> availableDrivers;

#if defined(OUZEL_SUPPORTS_OPENGL) || defined(OUZEL_SUPPORTS_OPENGLES)
        availableDrivers.insert(Renderer::Driver::OPENGL);
#elif defined(SUPPORTS_DIRECT3D11)
        availableDrivers.insert(Renderer::Driver::DIRECT3D11);
#endif

        return availableDrivers;
    }

    bool Engine::init()
    {
        _app = ouzelMain(_args);

        if (!_app)
        {
            return false;
        }

        Settings settings = _app->getSettings();
        _targetFPS = settings.targetFPS;

#if defined(OUZEL_PLATFORM_OSX) || defined(OUZEL_PLATFORM_IOS) || defined(OUZEL_PLATFORM_TVOS) || defined(OUZEL_PLATFORM_ANDROID) || defined(OUZEL_PLATFORM_LINUX)
        settings.driver = Renderer::Driver::OPENGL;
#elif defined(SUPPORTS_DIRECT3D11)
        settings.driver = Renderer::Driver::DIRECT3D11;
#endif

#if defined(OUZEL_PLATFORM_OSX)
        _window.reset(new WindowOSX(settings.size, settings.resizable, settings.fullscreen, settings.title));
#elif defined(OUZEL_PLATFORM_IOS)
        _window.reset(new WindowIOS(settings.size, settings.resizable, settings.fullscreen, settings.title));
#elif defined(OUZEL_PLATFORM_TVOS)
        _window.reset(new WindowTVOS(settings.size, settings.resizable, settings.fullscreen, settings.title));
#elif defined(OUZEL_PLATFORM_ANDROID)
        _window.reset(new WindowAndroid(settings.size, settings.resizable, settings.fullscreen, settings.title));
#elif defined(OUZEL_PLATFORM_LINUX)
        _window.reset(new WindowLinux(settings.size, settings.resizable, settings.fullscreen, settings.title));
#elif defined(OUZEL_PLATFORM_WINDOWS)
        _window.reset(new WindowWin(settings.size, settings.resizable, settings.fullscreen, settings.title));
#endif
        
        _eventDispatcher.reset(new EventDispatcher());
        _cache.reset(new Cache());
        _fileSystem.reset(new FileSystem());
        _sceneManager.reset(new SceneManager());

#if defined(OUZEL_PLATFORM_OSX) || defined(OUZEL_PLATFORM_IOS) || defined(OUZEL_PLATFORM_TVOS)
        _input.reset(new InputApple());
#elif defined(OUZEL_PLATFORM_WINDOWS)
        _input.reset(new InputWin());
#else
        _input.reset(new Input());
#endif

        switch (settings.driver)
        {
#if defined(OUZEL_SUPPORTS_OPENGL) || defined(OUZEL_SUPPORTS_OPENGLES)
            case Renderer::Driver::OPENGL:
                _renderer.reset(new RendererOGL());
				break;
#endif
#if defined(SUPPORTS_DIRECT3D11)
            case Renderer::Driver::DIRECT3D11:
                _renderer.reset(new RendererD3D11());
                break;
#endif
            default:
                _renderer.reset(new Renderer());
                break;
        }

        if (!_window->init())
        {
            return false;
        }

        _previousFrameTime = getCurrentMicroSeconds();

        return true;
    }

    void Engine::exit()
    {
        _active = false;
    }

    void Engine::begin()
    {
        _app->begin();
    }

    void Engine::end()
    {
        _app.reset();
        // remove the active scene
        _sceneManager->setScene(ScenePtr());
        _cache.reset();
        _renderer.reset();
        _window.reset();
    }

    bool Engine::run()
    {
        uint64_t currentTime = getCurrentMicroSeconds();
        float delta = static_cast<float>((currentTime - _previousFrameTime)) / 1000000.0f;
        _previousFrameTime = currentTime;

        _currentFPS = 1.0f / delta;

        _input->update();
        _eventDispatcher->update();

        lock();
        for (const UpdateCallbackPtr& updateCallback : _updateCallbacks)
        {
            if (!updateCallback->_remove && updateCallback->callback)
            {
                updateCallback->callback(delta);
            }
        }
        unlock();

        _renderer->clear();
        _sceneManager->draw();
        _renderer->present();

        return _active;
    }

    void Engine::scheduleUpdate(const UpdateCallbackPtr& callback)
    {
        if (_locked)
        {
            _updateCallbackAddList.insert(callback);
        }
        else
        {
            std::vector<UpdateCallbackPtr>::iterator i = std::find(_updateCallbacks.begin(), _updateCallbacks.end(), callback);

            if (i == _updateCallbacks.end())
            {
                callback->_remove = false;
                _updateCallbacks.push_back(callback);
            }
        }
    }

    void Engine::unscheduleUpdate(const UpdateCallbackPtr& callback)
    {
        if (_locked)
        {
            callback->_remove = true;
            _updateCallbackRemoveList.insert(callback);
        }
        else
        {
            std::vector<UpdateCallbackPtr>::iterator i = std::find(_updateCallbacks.begin(), _updateCallbacks.end(), callback);

            if (i != _updateCallbacks.end())
            {
                _updateCallbacks.erase(i);
            }
        }
    }

    void Engine::lock()
    {
        ++_locked;
    }

    void Engine::unlock()
    {
        if (--_locked == 0)
        {
            if (!_updateCallbackAddList.empty())
            {
                for (const UpdateCallbackPtr& updateCallback : _updateCallbackAddList)
                {
                    scheduleUpdate(updateCallback);
                }
                _updateCallbackAddList.clear();
            }

            if (!_updateCallbackRemoveList.empty())
            {
                for (const UpdateCallbackPtr& updateCallback : _updateCallbackRemoveList)
                {
                    unscheduleUpdate(updateCallback);
                }
                _updateCallbackRemoveList.clear();
            }
        }
    }
}
