// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <glob.h>
#include <linux/input.h>
#include "InputRasp.hpp"
#include "core/Engine.hpp"
#include "core/Window.hpp"
#include "utils/Log.hpp"

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define BITS_PER_LONG (8 * sizeof(long))
#define BITS_TO_LONGS(nr) DIV_ROUND_UP(nr, BITS_PER_LONG)

static inline int isBitSet(const unsigned long* array, int bit)
{
    return !!(array[bit / BITS_PER_LONG] & (1LL << (bit % BITS_PER_LONG)));
}

static char TEMP_BUFFER[256];

namespace ouzel
{
    namespace input
    {
        static const std::map<uint16_t, KeyboardKey> keyMap = {
            {KEY_ESC, KeyboardKey::ESCAPE},
            {KEY_1, KeyboardKey::NUM_1},
            {KEY_2, KeyboardKey::NUM_2},
            {KEY_3, KeyboardKey::NUM_3},
            {KEY_4, KeyboardKey::NUM_4},
            {KEY_5, KeyboardKey::NUM_5},
            {KEY_6, KeyboardKey::NUM_6},
            {KEY_7, KeyboardKey::NUM_7},
            {KEY_8, KeyboardKey::NUM_8},
            {KEY_9, KeyboardKey::NUM_9},
            {KEY_0, KeyboardKey::NUM_0},
            {KEY_MINUS, KeyboardKey::MINUS},
            {KEY_EQUAL, KeyboardKey::PLUS},
            {KEY_BACKSPACE, KeyboardKey::BACKSPACE},
            {KEY_TAB, KeyboardKey::TAB},
            {KEY_Q, KeyboardKey::Q},
            {KEY_W, KeyboardKey::W},
            {KEY_E, KeyboardKey::E},
            {KEY_R, KeyboardKey::R},
            {KEY_T, KeyboardKey::T},
            {KEY_Y, KeyboardKey::Y},
            {KEY_U, KeyboardKey::U},
            {KEY_I, KeyboardKey::I},
            {KEY_O, KeyboardKey::O},
            {KEY_P, KeyboardKey::P},
            {KEY_LEFTBRACE, KeyboardKey::BRACKET_LEFT},
            {KEY_RIGHTBRACE, KeyboardKey::BRACKET_RIGHT},
            {KEY_ENTER, KeyboardKey::RETURN},
            {KEY_LEFTCTRL, KeyboardKey::LCONTROL},
            {KEY_A, KeyboardKey::A},
            {KEY_S, KeyboardKey::S},
            {KEY_D, KeyboardKey::D},
            {KEY_F, KeyboardKey::F},
            {KEY_G, KeyboardKey::G},
            {KEY_H, KeyboardKey::H},
            {KEY_J, KeyboardKey::J},
            {KEY_K, KeyboardKey::K},
            {KEY_L, KeyboardKey::L},
            {KEY_SEMICOLON, KeyboardKey::SEMICOLON},
            {KEY_APOSTROPHE, KeyboardKey::QUOTE},
            {KEY_GRAVE, KeyboardKey::GRAVE},
            {KEY_LEFTSHIFT, KeyboardKey::LSHIFT},
            {KEY_BACKSLASH, KeyboardKey::BACKSLASH},
            {KEY_Z, KeyboardKey::Z},
            {KEY_X, KeyboardKey::X},
            {KEY_C, KeyboardKey::C},
            {KEY_V, KeyboardKey::V},
            {KEY_B, KeyboardKey::B},
            {KEY_N, KeyboardKey::N},
            {KEY_M, KeyboardKey::M},
            {KEY_COMMA, KeyboardKey::COMMA},
            {KEY_DOT, KeyboardKey::PERIOD},
            {KEY_SLASH, KeyboardKey::SLASH},
            {KEY_RIGHTSHIFT, KeyboardKey::RSHIFT},
            {KEY_KPASTERISK, KeyboardKey::MULTIPLY},
            {KEY_LEFTALT, KeyboardKey::LALT},
            {KEY_SPACE, KeyboardKey::SPACE},
            {KEY_CAPSLOCK, KeyboardKey::CAPITAL},
            {KEY_F1, KeyboardKey::F1},
            {KEY_F2, KeyboardKey::F2},
            {KEY_F3, KeyboardKey::F3},
            {KEY_F4, KeyboardKey::F4},
            {KEY_F5, KeyboardKey::F5},
            {KEY_F6, KeyboardKey::F6},
            {KEY_F7, KeyboardKey::F7},
            {KEY_F8, KeyboardKey::F8},
            {KEY_F9, KeyboardKey::F9},
            {KEY_F10, KeyboardKey::F10},
            {KEY_NUMLOCK, KeyboardKey::NUMLOCK},
            {KEY_SCROLLLOCK, KeyboardKey::SCROLL},
            {KEY_KP7, KeyboardKey::NUMPAD_7},
            {KEY_KP8, KeyboardKey::NUMPAD_8},
            {KEY_KP9, KeyboardKey::NUMPAD_9},
            {KEY_KPMINUS, KeyboardKey::SUBTRACT},
            {KEY_KP4, KeyboardKey::NUMPAD_4},
            {KEY_KP5, KeyboardKey::NUMPAD_5},
            {KEY_KP6, KeyboardKey::NUMPAD_6},
            {KEY_KPPLUS, KeyboardKey::ADD},
            {KEY_KP1, KeyboardKey::NUMPAD_1},
            {KEY_KP2, KeyboardKey::NUMPAD_2},
            {KEY_KP3, KeyboardKey::NUMPAD_3},
            {KEY_KP0, KeyboardKey::NUMPAD_0},
            {KEY_KPDOT, KeyboardKey::DECIMAL},

            {KEY_ZENKAKUHANKAKU, KeyboardKey::NONE}, // ??
            {KEY_102ND, KeyboardKey::LESS},
            {KEY_F11, KeyboardKey::F11},
            {KEY_F12, KeyboardKey::F12},
            {KEY_RO, KeyboardKey::NONE}, // ??
            {KEY_KATAKANA, KeyboardKey::NONE}, // ??
            {KEY_HIRAGANA, KeyboardKey::NONE}, // ??
            {KEY_HENKAN, KeyboardKey::NONE}, // ??
            {KEY_KATAKANAHIRAGANA, KeyboardKey::NONE}, // ??
            {KEY_MUHENKAN, KeyboardKey::NONE}, // ??
            {KEY_KPJPCOMMA, KeyboardKey::NONE}, // ??
            {KEY_KPENTER, KeyboardKey::RETURN},
            {KEY_RIGHTCTRL, KeyboardKey::RCONTROL},
            {KEY_KPSLASH, KeyboardKey::DIVIDE},
            {KEY_SYSRQ, KeyboardKey::NONE}, // ??
            {KEY_RIGHTALT, KeyboardKey::RALT},
            {KEY_LINEFEED, KeyboardKey::NONE}, // ??
            {KEY_HOME, KeyboardKey::HOME},
            {KEY_UP, KeyboardKey::UP},
            {KEY_PAGEUP, KeyboardKey::PRIOR},
            {KEY_LEFT, KeyboardKey::LEFT},
            {KEY_RIGHT, KeyboardKey::RIGHT},
            {KEY_END, KeyboardKey::END},
            {KEY_DOWN, KeyboardKey::DOWN},
            {KEY_PAGEDOWN, KeyboardKey::NEXT},
            {KEY_INSERT, KeyboardKey::INSERT},
            {KEY_DELETE, KeyboardKey::DEL},
            {KEY_MACRO, KeyboardKey::NONE}, // ??
            {KEY_MUTE, KeyboardKey::NONE}, // ??
            {KEY_VOLUMEDOWN, KeyboardKey::NONE}, // ??
            {KEY_VOLUMEUP, KeyboardKey::NONE}, // ??
            {KEY_POWER, KeyboardKey::NONE}, // ??
            {KEY_KPEQUAL, KeyboardKey::EQUAL},
            {KEY_KPPLUSMINUS, KeyboardKey::NONE}, // ??
            {KEY_PAUSE, KeyboardKey::PAUSE},
            {KEY_SCALE, KeyboardKey::NONE}, //?

            {KEY_KPCOMMA, KeyboardKey::SEPARATOR},
            {KEY_HANGEUL, KeyboardKey::NONE}, // KEY_HANGUEL
            {KEY_HANJA, KeyboardKey::HANJA},
            {KEY_YEN, KeyboardKey::NONE}, // ??
            {KEY_LEFTMETA, KeyboardKey::LSUPER},
            {KEY_RIGHTMETA, KeyboardKey::RSUPER},
            {KEY_COMPOSE, KeyboardKey::NONE}, // ??

            {KEY_F13, KeyboardKey::F13},
            {KEY_F14, KeyboardKey::F14},
            {KEY_F15, KeyboardKey::F15},
            {KEY_F16, KeyboardKey::F16},
            {KEY_F17, KeyboardKey::F17},
            {KEY_F18, KeyboardKey::F18},
            {KEY_F19, KeyboardKey::F19},
            {KEY_F20, KeyboardKey::F20},
            {KEY_F21, KeyboardKey::F21},
            {KEY_F22, KeyboardKey::F22},
            {KEY_F23, KeyboardKey::F23},
            {KEY_F24, KeyboardKey::F24}
        };

        static KeyboardKey convertKeyCode(uint16_t keyCode)
        {
            auto i = keyMap.find(keyCode);

            if (i != keyMap.end())
            {
                return i->second;
            }
            else
            {
                return KeyboardKey::NONE;
            }
        }

        InputRasp::InputRasp()
        {
        }

        bool InputRasp::init()
        {
            std::fill(std::begin(keyboardKeyDown), std::end(keyboardKeyDown), false);
            std::fill(std::begin(mouseButtonDown), std::end(mouseButtonDown), false);

            glob_t g;
            int result = glob("/dev/input/event*", GLOB_NOSORT, NULL, &g);

            if (result == GLOB_NOMATCH)
            {
                Log(Log::Level::WARN) << "No event devices found";
                return false;
            }
            else if (result)
            {
                Log(Log::Level::ERR) << "Could not read /dev/input/event*";
                return false;
            }

            for (size_t i = 0; i < g.gl_pathc; i++)
            {
                InputDeviceRasp inputDevice;

                inputDevice.fd = open(g.gl_pathv[i], O_RDONLY);
                if (inputDevice.fd == -1)
                {
                    Log(Log::Level::WARN) << "Failed to open device file descriptor";
                    continue;
                }

                if (ioctl(inputDevice.fd, EVIOCGRAB, (void *)1) == -1)
                {
                    Log(Log::Level::WARN) << "Failed to get grab device";
                }

                std::fill(std::begin(TEMP_BUFFER), std::end(TEMP_BUFFER), 0);
                if (ioctl(inputDevice.fd, EVIOCGNAME(sizeof(TEMP_BUFFER) - 1), TEMP_BUFFER) == -1)
                {
                    Log(Log::Level::WARN) << "Failed to get device name";
                }
                else
                {
                    Log(Log::Level::INFO) << "Got device: " << TEMP_BUFFER;
                }

                unsigned long eventBits[BITS_TO_LONGS(EV_CNT)];
                unsigned long absBits[BITS_TO_LONGS(ABS_CNT)];
                unsigned long relBits[BITS_TO_LONGS(REL_CNT)];
                unsigned long keyBits[BITS_TO_LONGS(KEY_CNT)];

                if (ioctl(inputDevice.fd, EVIOCGBIT(0, sizeof(eventBits)), eventBits) == -1 ||
                    ioctl(inputDevice.fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits) == -1 ||
                    ioctl(inputDevice.fd, EVIOCGBIT(EV_REL, sizeof(relBits)), relBits) == -1 ||
                    ioctl(inputDevice.fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits) == -1)
                {
                    Log(Log::Level::WARN) << "Failed to get device event bits";
                    continue;
                }

                if (isBitSet(eventBits, EV_KEY) && (
                     isBitSet(keyBits, KEY_1) ||
                     isBitSet(keyBits, KEY_2) ||
                     isBitSet(keyBits, KEY_3) ||
                     isBitSet(keyBits, KEY_4) ||
                     isBitSet(keyBits, KEY_5) ||
                     isBitSet(keyBits, KEY_6) ||
                     isBitSet(keyBits, KEY_7) ||
                     isBitSet(keyBits, KEY_8) ||
                     isBitSet(keyBits, KEY_9) ||
                     isBitSet(keyBits, KEY_0)
                    ))
                {
                    Log(Log::Level::INFO) << "Device class: keyboard";
                    inputDevice.deviceClass = InputDeviceRasp::CLASS_KEYBOARD;
                }

                if (isBitSet(eventBits, EV_ABS) && isBitSet(absBits, ABS_X) && isBitSet(absBits, ABS_Y))
                {
                    if (isBitSet(keyBits, BTN_STYLUS) || isBitSet(keyBits, BTN_TOOL_PEN))
                    {
                        Log(Log::Level::INFO) << "Device class: tablet";
                        inputDevice.deviceClass |= InputDeviceRasp::CLASS_TOUCHPAD;
                    }
                    else if (isBitSet(keyBits, BTN_TOOL_FINGER) && !isBitSet(keyBits, BTN_TOOL_PEN))
                    {
                        Log(Log::Level::INFO) << "Device class: touchpad";
                        inputDevice.deviceClass |= InputDeviceRasp::CLASS_TOUCHPAD;
                    }
                    else if (isBitSet(keyBits, BTN_MOUSE))
                    {
                        Log(Log::Level::INFO) << "Device class: mouse";
                        inputDevice.deviceClass |= InputDeviceRasp::CLASS_MOUSE;
                    }
                    else if (isBitSet(keyBits, BTN_TOUCH))
                    {
                        Log(Log::Level::INFO) << "Device class: touchscreen";
                        inputDevice.deviceClass |= InputDeviceRasp::CLASS_TOUCHPAD;
                    }
                }
                else if (isBitSet(eventBits, EV_REL) && isBitSet(relBits, REL_X) && isBitSet(relBits, REL_Y))
                {
                    if (isBitSet(keyBits, BTN_MOUSE))
                    {
                        Log(Log::Level::INFO) << "Device class: mouse";
                        inputDevice.deviceClass |= InputDeviceRasp::CLASS_MOUSE;
                    }
                }

                if (isBitSet(keyBits, BTN_JOYSTICK))
                {
                    Log(Log::Level::INFO) << "Device class: joystick";
                    inputDevice.deviceClass = InputDeviceRasp::CLASS_GAMEPAD;
                }

                if (isBitSet(keyBits, BTN_GAMEPAD))
                {
                    Log(Log::Level::INFO) << "Device class: gamepad";
                    inputDevice.deviceClass = InputDeviceRasp::CLASS_GAMEPAD;
                }

                if (inputDevice.fd > maxFd)
                {
                    maxFd = inputDevice.fd;
                }

                inputDevices.push_back(inputDevice);
            }

            globfree(&g);

            return true;
        }

        InputRasp::~InputRasp()
        {
            for (const InputDeviceRasp& inputDevice : inputDevices)
            {
                if (ioctl(inputDevice.fd, EVIOCGRAB, (void*)0) == -1)
                {
                    Log(Log::Level::WARN) << "Failed to release device";
                }

                if (close(inputDevice.fd) == -1)
                {
                    Log(Log::Level::ERR) << "Failed to close file descriptor";
                }
            }
        }

        void InputRasp::update()
        {
            fd_set rfds;
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 0;

            FD_ZERO(&rfds);

            for (const InputDeviceRasp& inputDevice : inputDevices)
            {
                FD_SET(inputDevice.fd, &rfds);
            }

            int retval = select(maxFd + 1, &rfds, NULL, NULL, &tv);

            if (retval == -1)
            {
                Log(Log::Level::ERR) << "Select failed";
            }
            else if (retval > 0)
            {
                for (const InputDeviceRasp& inputDevice : inputDevices)
                {
                    if (FD_ISSET(inputDevice.fd, &rfds))
                    {
                        ssize_t bytesRead = read(inputDevice.fd, TEMP_BUFFER, sizeof(TEMP_BUFFER));

                        if (bytesRead == -1)
                        {
                            continue;
                        }

                        for (ssize_t i = 0; i < bytesRead - static_cast<ssize_t>(sizeof(input_event)) + 1; i += sizeof(input_event))
                        {
                            input_event* event = reinterpret_cast<input_event*>(TEMP_BUFFER + i);

                            if (inputDevice.deviceClass & InputDeviceRasp::CLASS_KEYBOARD)
                            {
                                if (event->type == EV_KEY)
                                {
                                    if (event->value == 1 || event->value == 2) // press or repeat
                                    {
                                        if (event->value >= 0 && event->value < 256) keyboardKeyDown[event->value] = true;
                                        keyPress(convertKeyCode(event->code), getModifiers());
                                    }
                                    else if (event->value == 0) // release
                                    {
                                        if (event->value >= 0 && event->value < 256) keyboardKeyDown[event->value] = false;
                                        keyRelease(convertKeyCode(event->code), getModifiers());
                                    }
                                }
                            }
                            if (inputDevice.deviceClass & InputDeviceRasp::CLASS_MOUSE)
                            {
                                if (event->type == EV_ABS)
                                {
                                    Vector2 absolutePos = cursorPosition;

                                    if (event->code == ABS_X)
                                    {
                                        absolutePos.v[0] = sharedEngine->getWindow()->convertWindowToNormalizedLocation(Vector2(static_cast<float>(event->value), 0.0f)).v[0];
                                    }
                                    else if (event->code == ABS_Y)
                                    {
                                        absolutePos.v[1] = sharedEngine->getWindow()->convertWindowToNormalizedLocation(Vector2(0.0f, static_cast<float>(event->value))).v[1];
                                    }

                                    mouseMove(absolutePos, getModifiers());
                                }
                                else if (event->type == EV_REL)
                                {
                                    Vector2 relativePos;

                                    if (event->code == REL_X)
                                    {
                                        relativePos.v[0] = static_cast<float>(event->value);
                                    }
                                    else if (event->code == REL_Y)
                                    {
                                        relativePos.v[1] = static_cast<float>(event->value);
                                    }

                                    mouseRelativeMove(sharedEngine->getWindow()->convertWindowToNormalizedLocationRelative(relativePos), getModifiers());
                                }
                                else if (event->type == EV_KEY)
                                {
                                    MouseButton button;
                                    int buttonIndex = -1;

                                    switch (event->code)
                                    {
                                    case BTN_LEFT:
                                        button = MouseButton::LEFT;
                                        buttonIndex = 0;
                                        break;
                                    case BTN_RIGHT:
                                        button = MouseButton::RIGHT;
                                        buttonIndex =  1;
                                        break;
                                    case BTN_MIDDLE:
                                        button = MouseButton::MIDDLE;
                                        buttonIndex = 2;
                                        break;
                                    default:
                                        button = MouseButton::NONE;
                                    }

                                    if (event->value == 1)
                                    {
                                        if (buttonIndex >= 0 && buttonIndex < 3) mouseButtonDown[buttonIndex] = true;
                                        mouseButtonPress(button, cursorPosition, getModifiers());
                                    }
                                    else if (event->value == 0)
                                    {
                                        if (buttonIndex >= 0 && buttonIndex < 3) mouseButtonDown[buttonIndex] = false;
                                        mouseButtonRelease(button, cursorPosition, getModifiers());
                                    }
                                }
                            }
                            if (inputDevice.deviceClass & InputDeviceRasp::CLASS_TOUCHPAD)
                            {
                                // TODO: implement
                            }
                            if (inputDevice.deviceClass & InputDeviceRasp::CLASS_GAMEPAD)
                            {
                                // TODO: implement
                            }
                        }
                    }
                }
            }
        }

        uint32_t InputRasp::getModifiers() const
        {
            uint32_t modifiers = 0;

            if (keyboardKeyDown[KEY_LEFTSHIFT] || keyboardKeyDown[KEY_RIGHTSHIFT]) modifiers |= SHIFT_DOWN;
            if (keyboardKeyDown[KEY_LEFTALT] || keyboardKeyDown[KEY_RIGHTALT]) modifiers |= ALT_DOWN;
            if (keyboardKeyDown[KEY_LEFTCTRL] || keyboardKeyDown[KEY_RIGHTCTRL]) modifiers |= CONTROL_DOWN;
            if (keyboardKeyDown[KEY_LEFTMETA] || keyboardKeyDown[KEY_RIGHTMETA]) modifiers |= SUPER_DOWN;

            if (mouseButtonDown[0]) modifiers |= LEFT_MOUSE_DOWN;
            if (mouseButtonDown[1]) modifiers |= RIGHT_MOUSE_DOWN;
            if (mouseButtonDown[2]) modifiers |= MIDDLE_MOUSE_DOWN;

            return modifiers;
        }
    } // namespace input
} // namespace ouzel
