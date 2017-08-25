// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <windowsx.h>
#include "WindowWin.hpp"
#include "core/Engine.hpp"
#include "core/Window.hpp"
#include "input/windows/InputWin.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/RenderDevice.hpp"
#include "graphics/direct3d11/RenderDeviceD3D11.hpp"
#include "utils/Log.hpp"

static void handleKeyEvent(UINT msg, WPARAM wParam, LPARAM lParam)
{
    UINT key = static_cast<UINT>(wParam);
    switch (key)
    {
        case VK_MENU:
            if((lParam & 0x1000000) == 0)
                key = VK_LMENU;
            else
                key = VK_RMENU;
            break;
        case VK_CONTROL:
            if((lParam & 0x1000000) == 0)
                key = VK_LCONTROL;
            else
                key = VK_RCONTROL;
            break;
        case VK_SHIFT:
            key = MapVirtualKey((lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
            break;
    }

    if (msg == WM_KEYDOWN)
    {
        ouzel::sharedEngine->getInput()->keyPress(ouzel::input::InputWin::convertKeyCode(key), 0);
    }
    else if (msg == WM_KEYUP)
    {
        ouzel::sharedEngine->getInput()->keyRelease(ouzel::input::InputWin::convertKeyCode(key), 0);
    }
}

static void handleMouseMoveEvent(UINT, WPARAM wParam, LPARAM lParam)
{
    ouzel::Vector2 position(static_cast<float>(GET_X_LPARAM(lParam)),
                            static_cast<float>(GET_Y_LPARAM(lParam)));

    ouzel::sharedEngine->getInput()->mouseMove(ouzel::sharedEngine->getWindow()->convertWindowToNormalizedLocation(position),
                                               ouzel::input::InputWin::getModifiers(wParam));
}

static void handleMouseButtonEvent(UINT msg, WPARAM wParam, LPARAM lParam)
{
    ouzel::Vector2 position(static_cast<float>(GET_X_LPARAM(lParam)),
                            static_cast<float>(GET_Y_LPARAM(lParam)));

    ouzel::input::MouseButton button;

    if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP)
    {
        button = ouzel::input::MouseButton::LEFT;
    }
    else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
    {
        button = ouzel::input::MouseButton::RIGHT;
    }
    else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP)
    {
        button = ouzel::input::MouseButton::MIDDLE;
    }
    else if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP)
    {
        if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
            button = ouzel::input::MouseButton::X1;
        else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2)
            button = ouzel::input::MouseButton::X2;
        else
            return;
    }
    else
    {
        return;
    }

    if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN)
    {
        ouzel::sharedEngine->getInput()->mouseButtonPress(button,
                                                          ouzel::sharedEngine->getWindow()->convertWindowToNormalizedLocation(position),
                                                          ouzel::input::InputWin::getModifiers(wParam));
    }
    else if (msg == WM_LBUTTONUP || msg == WM_RBUTTONUP || msg == WM_MBUTTONUP || msg == WM_XBUTTONUP)
    {
        ouzel::sharedEngine->getInput()->mouseButtonRelease(button,
                                                            ouzel::sharedEngine->getWindow()->convertWindowToNormalizedLocation(position),
                                                            ouzel::input::InputWin::getModifiers(wParam));
    }
}

static void handleMouseWheelEvent(UINT msg, WPARAM wParam, LPARAM lParam)
{
    ouzel::Vector2 position(static_cast<float>(GET_X_LPARAM(lParam)),
                            static_cast<float>(GET_Y_LPARAM(lParam)));

    if (msg == WM_MOUSEWHEEL)
    {
        short param = static_cast<short>(HIWORD(wParam));
        ouzel::sharedEngine->getInput()->mouseScroll(ouzel::Vector2(0.0f, -static_cast<float>(param) / static_cast<float>(WHEEL_DELTA)),
                                                     ouzel::sharedEngine->getWindow()->convertWindowToNormalizedLocation(position),
                                                     ouzel::input::InputWin::getModifiers(wParam));
    }
    else if (msg == WM_MOUSEHWHEEL)
    {
        short param = static_cast<short>(HIWORD(wParam));
        ouzel::sharedEngine->getInput()->mouseScroll(ouzel::Vector2(static_cast<float>(param) / static_cast<float>(WHEEL_DELTA), 0.0f),
                                                     ouzel::sharedEngine->getWindow()->convertWindowToNormalizedLocation(position),
                                                     ouzel::input::InputWin::getModifiers(wParam));
    }
}

static void handleTouchEvent(WPARAM wParam, LPARAM lParam)
{
    UINT inputCount = LOWORD(wParam);
    std::vector<TOUCHINPUT> touches(inputCount);

    if (GetTouchInputInfo(reinterpret_cast<HTOUCHINPUT>(lParam), inputCount, touches.data(), sizeof(TOUCHINPUT)))
    {
        ouzel::Vector2 position;

        for (const TOUCHINPUT& touch : touches)
        {
            position.x = static_cast<float>(touch.x / 100);
            position.y = static_cast<float>(touch.y / 100);

            if (touch.dwFlags & TOUCHEVENTF_DOWN)
            {
                ouzel::sharedEngine->getInput()->touchBegin(touch.dwID,
                                                            ouzel::sharedEngine->getWindow()->convertWindowToNormalizedLocation(position));
            }

            if (touch.dwFlags & TOUCHEVENTF_UP)
            {
                ouzel::sharedEngine->getInput()->touchEnd(touch.dwID,
                                                          ouzel::sharedEngine->getWindow()->convertWindowToNormalizedLocation(position));
            }

            if (touch.dwFlags & TOUCHEVENTF_MOVE)
            {
                ouzel::sharedEngine->getInput()->touchMove(touch.dwID,
                                                           ouzel::sharedEngine->getWindow()->convertWindowToNormalizedLocation(position));
            }
        }

        if (!CloseTouchInputHandle(reinterpret_cast<HTOUCHINPUT>(lParam)))
        {
            ouzel::Log(ouzel::Log::Level::ERR) << "Failed to close touch input handle";
        }
    }
    else
    {
        ouzel::Log(ouzel::Log::Level::ERR) << "Failed to get touch info";
    }
}

static const LONG_PTR SIGNATURE_MASK = 0x0FFFFFF00;
static const LONG_PTR MOUSEEVENTF_FROMTOUCH = 0x0FF515700;

static LRESULT CALLBACK windowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ouzel::WindowWin* windowWin = reinterpret_cast<ouzel::WindowWin*>(GetWindowLongPtr(window, GWLP_USERDATA));
    if (!windowWin) return DefWindowProcW(window, msg, wParam, lParam);

    switch (msg)
    {
        case WM_ACTIVATEAPP:
        {
            if (wParam)
            {
                if (ouzel::sharedEngine) ouzel::sharedEngine->resume();
            }
            else
            {
                if (ouzel::sharedEngine) ouzel::sharedEngine->pause();
            }
            break;
        }
        case WM_KEYUP:
        case WM_KEYDOWN:
        {
            handleKeyEvent(msg, wParam, lParam);
            break;
        }
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        {
            LONG_PTR extraInfo = GetMessageExtraInfo();

            // don't handle mouse event that came from touch
            if ((extraInfo & SIGNATURE_MASK) != MOUSEEVENTF_FROMTOUCH)
            {
                handleMouseButtonEvent(msg, wParam, lParam);

                if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP)
                    return TRUE;

                return 0;
            }
        }
        case WM_MOUSEMOVE:
        {
            LONG_PTR extraInfo = GetMessageExtraInfo();

            // don't handle mouse event that came from touch
            if ((extraInfo & SIGNATURE_MASK) != MOUSEEVENTF_FROMTOUCH)
            {
                handleMouseMoveEvent(msg, wParam, lParam);
                return 0;
            }
        }
        case WM_MOUSEWHEEL:
        case WM_MOUSEHWHEEL:
        {
            handleMouseWheelEvent(msg, wParam, lParam);
            return 0;
        }
        case WM_TOUCH:
        {
            handleTouchEvent(wParam, lParam);
            return 0;
        }
        case WM_SETCURSOR:
        {
            if (LOWORD(lParam) == HTCLIENT)
            {
                ouzel::input::InputWin* inputWin = static_cast<ouzel::input::InputWin*>(ouzel::sharedEngine->getInput());
                inputWin->updateCursor();
                return TRUE;
            }
            break;
        }
        case WM_SIZE:
        {
            switch (wParam)
            {
                case SIZE_MINIMIZED:
                    ouzel::sharedEngine->pause();
                    break;
                case SIZE_RESTORED:
                    ouzel::sharedEngine->resume();
                    // fall through
                case SIZE_MAXIMIZED:
                    windowWin->handleResize(ouzel::Size2(static_cast<float>(LOWORD(lParam)),
                                                         static_cast<float>(HIWORD(lParam))));
                    break;
            }
            return 0;
        }
        case WM_MOVE:
        {
            windowWin->handleMove();
            break;
        }
        case WM_ERASEBKGND:
        {
            // Erase background only for the Empty renderer
            if (ouzel::sharedEngine->getRenderer()->getDevice()->getDriver() != ouzel::graphics::Renderer::Driver::EMPTY)
            {
                return TRUE;
            }
            break;
        }
        case WM_SYSCOMMAND:
        {
            switch (wParam)
            {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                {
                    if (!ouzel::sharedEngine->isScreenSaverEnabled())
                    {
                        // Disable screensaver
                        return 0;
                    }
                    break;
                }
                // Disable accessing menu using alt key
                case SC_KEYMENU:
                    return 0;
            }
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcW(window, msg, wParam, lParam);
}

static const LPCWSTR WINDOW_CLASS_NAME = L"OuzelWindow";

namespace ouzel
{
    WindowWin::WindowWin()
    {
    }

    WindowWin::~WindowWin()
    {
        if (window)
        {
            DestroyWindow(window);
        }

        if (windowClass)
        {
            UnregisterClassW(WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
        }
    }

    bool WindowWin::init(const Size2& newSize,
                         bool newResizable,
                         bool newFullscreen,
                         const std::string& newTitle,
                         bool newHighDpi,
                         bool depth)
    {
        if (!Window::init(newSize, newResizable, newFullscreen, newTitle, newHighDpi, depth))
        {
            return false;
        }

        if (highDpi)
        {
            HMODULE shcore = LoadLibraryW(L"shcore.dll");

            if (shcore)
            {
                typedef HRESULT(STDAPICALLTYPE *SetProcessDpiAwarenessProc)(int value);
                SetProcessDpiAwarenessProc setProcessDpiAwareness = reinterpret_cast<SetProcessDpiAwarenessProc>(GetProcAddress(shcore, "SetProcessDpiAwareness"));

                if (setProcessDpiAwareness)
                {
                    setProcessDpiAwareness(2); // PROCESS_PER_MONITOR_DPI_AWARE
                }
            }
        }

        HINSTANCE instance = GetModuleHandleW(nullptr);

        WNDCLASSEXW wc;
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = windowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = instance;
        // Application icon should be the first resource
        wc.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(101));
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        if (sharedEngine->getRenderer()->getDevice()->getDriver() == graphics::Renderer::Driver::EMPTY)
        {
            wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(COLOR_WINDOW));
        }
        else
        {
            wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        }
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = WINDOW_CLASS_NAME;
        wc.hIconSm = nullptr;

        windowClass = RegisterClassExW(&wc);
        if (!windowClass)
        {
            Log(Log::Level::ERR) << "Failed to register window class";
            return false;
        }

        windowWindowedStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPSIBLINGS | WS_BORDER | WS_DLGFRAME | WS_THICKFRAME | WS_GROUP | WS_TABSTOP;

        if (resizable)
        {
            windowWindowedStyle |= WS_SIZEBOX | WS_MAXIMIZEBOX;
        }

        windowFullscreenStyle = WS_CLIPSIBLINGS | WS_GROUP | WS_TABSTOP;

        int x = CW_USEDEFAULT;
        int y = CW_USEDEFAULT;

        windowStyle = windowWindowedStyle;

        RECT windowRect = {0, 0, static_cast<LONG>(size.width), static_cast<LONG>(size.height)};
        AdjustWindowRect(&windowRect, windowStyle, FALSE);

        int width = CW_USEDEFAULT;
        int height = CW_USEDEFAULT;
        if (size.width > 0.0f) width = windowRect.right - windowRect.left;
        if (size.height > 0.0f) height = windowRect.bottom - windowRect.top;

        wchar_t titleBuffer[256] = L"";

        if (!title.empty() && MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, titleBuffer, 256) == 0)
        {
            Log(Log::Level::ERR) << "Failed to convert UTF-8 to wide char";
            return false;
        }

        window = CreateWindowExW(WS_EX_APPWINDOW, WINDOW_CLASS_NAME, titleBuffer, windowStyle,
                                 x, y, width, height, nullptr, nullptr, instance, nullptr);

        if (!window)
        {
            Log(Log::Level::ERR) << "Failed to create window";
            return false;
        }

        monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);

        if (fullscreen)
        {
            switchFullscreen(fullscreen);
        }

        GetClientRect(window, &windowRect);
        size.width = static_cast<float>(windowRect.right - windowRect.left);
        size.height = static_cast<float>(windowRect.bottom - windowRect.top);

        if (!RegisterTouchWindow(window, 0))
        {
            Log(Log::Level::WARN) << "Failed to enable touch for window";
        }

        ShowWindow(window, SW_SHOW);
        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        return true;
    }

    void WindowWin::close()
    {
        Window::close();

        sharedEngine->executeOnMainThread([this] {
            SendMessage(window, WM_CLOSE, 0, 0);
        });
    }

    void WindowWin::setSize(const Size2& newSize)
    {
        sharedEngine->executeOnMainThread([this, newSize] {
            UINT width = static_cast<UINT>(newSize.width);
            UINT height = static_cast<UINT>(newSize.height);

            UINT swpFlags = SWP_NOMOVE | SWP_NOZORDER;

            RECT rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
            AdjustWindowRect(&rect, windowStyle, GetMenu(window) ? TRUE : FALSE);

            SetWindowPos(window, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, swpFlags);
        });

        Window::setSize(newSize);
    }

    void WindowWin::setTitle(const std::string& newTitle)
    {
        if (title != newTitle)
        {
            sharedEngine->executeOnMainThread([this, newTitle] {
                wchar_t titleBuffer[256] = L"";

                if (!newTitle.empty() && MultiByteToWideChar(CP_UTF8, 0, newTitle.c_str(), -1, titleBuffer, 256) == 0)
                {
                    Log(Log::Level::ERR) << "Failed to convert UTF-8 to wide char";
                    return;
                }

                SetWindowTextW(window, titleBuffer);
            });
        }

        Window::setTitle(newTitle);
    }

    void WindowWin::setFullscreen(bool newFullscreen)
    {
        Window::setFullscreen(newFullscreen);

        sharedEngine->executeOnMainThread([this, newFullscreen] {
            switchFullscreen(newFullscreen);
        });
    }

    void WindowWin::switchFullscreen(bool newFullscreen)
    {
        windowStyle = (newFullscreen ? windowFullscreenStyle : windowWindowedStyle) | WS_VISIBLE;
        SetWindowLong(window, GWL_STYLE, windowStyle);

        if (newFullscreen)
        {
            RECT windowRect;
            GetWindowRect(window, &windowRect);

            windowX = windowRect.left;
            windowY = windowRect.top;
            windowWidth = windowRect.right - windowRect.left;
            windowHeight = windowRect.bottom - windowRect.top;

            MONITORINFO info;
            info.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo(monitor, &info);

            SetWindowPos(window, nullptr, info.rcMonitor.left, info.rcMonitor.top,
                         info.rcMonitor.right - info.rcMonitor.left,
                         info.rcMonitor.bottom - info.rcMonitor.top,
                         SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
        }
        else
        {
            SetWindowPos(window, nullptr, windowX, windowY, windowWidth, windowHeight,
                         SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
        }
    }

    void WindowWin::handleResize(const Size2& newSize)
    {
        monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);

        Event event;
        event.type = Event::Type::WINDOW_SIZE_CHANGE;

        event.windowEvent.window = this;
        event.windowEvent.size = newSize;

        sharedEngine->getEventDispatcher()->postEvent(event);
    }

    void WindowWin::handleMove()
    {
        monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
    }

    void WindowWin::addAccelerator(HACCEL accelerator)
    {
        sharedEngine->executeOnMainThread([this, accelerator]() {
            accelerators.insert(accelerator);
        });
    }

    void WindowWin::removeAccelerator(HACCEL accelerator)
    {
        sharedEngine->executeOnMainThread([this, accelerator]() {
            accelerators.erase(accelerator);
        });
    }
}
