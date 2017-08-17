// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <X11/cursorfont.h>
#include <X11/Xcursor/Xcursor.h>
#include "CursorResourceLinux.hpp"
#include "core/Engine.hpp"
#include "core/linux/WindowLinux.hpp"
#include "utils/Log.hpp"

namespace ouzel
{
    namespace input
    {
        CursorResourceLinux::~CursorResourceLinux()
        {
            if (sharedEngine)
            {
                WindowLinux* windowLinux = static_cast<WindowLinux*>(sharedEngine->getWindow());
                Display* display = windowLinux->getDisplay();
                if (cursor != None) XFreeCursor(display, cursor);
            }
        }

        bool CursorResourceLinux::init(SystemCursor newSystemCursor)
        {
            if (!CursorResource::init(newSystemCursor))
            {
                return false;
            }

            WindowLinux* windowLinux = static_cast<WindowLinux*>(sharedEngine->getWindow());
            Display* display = windowLinux->getDisplay();

            if (cursor != None)
            {
                XFreeCursor(display, cursor);
                cursor = None;
            }

            switch (systemCursor)
            {
                case SystemCursor::DEFAULT:
                    cursor = XcursorLibraryLoadCursor(display, "arrow");
                    break;
                case SystemCursor::ARROW:
                    cursor = XcursorLibraryLoadCursor(display, "arrow");
                    break;
                case SystemCursor::HAND:
                    cursor = XcursorLibraryLoadCursor(display, "hand1");
                    break;
                case SystemCursor::HORIZONTAL_RESIZE:
                    cursor = XcursorLibraryLoadCursor(display, "sb_h_double_arrow");
                    break;
                case SystemCursor::VERTICAL_RESIZE:
                    cursor = XcursorLibraryLoadCursor(display, "sb_v_double_arrow");
                    break;
                case SystemCursor::CROSS:
                    cursor = XcursorLibraryLoadCursor(display, "crosshair");
                    break;
                case SystemCursor::I_BEAM:
                    cursor = XcursorLibraryLoadCursor(display, "xterm");
                    break;
            }

            reactivate();

            return true;
        }

        bool CursorResourceLinux::init(const std::vector<uint8_t>& newData,
                                       const Size2& newSize,
                                       graphics::PixelFormat newPixelFormat,
                                       const Vector2& newHotSpot)
        {
            if (!CursorResource::init(newData,
                                      newSize,
                                      newPixelFormat,
                                      newHotSpot))
            {
                return false;
            }

            WindowLinux* windowLinux = static_cast<WindowLinux*>(sharedEngine->getWindow());
            Display* display = windowLinux->getDisplay();

            if (cursor != None)
            {
                XFreeCursor(display, cursor);
                cursor = None;
            }

            if (!data.empty())
            {
                int width = static_cast<int>(size.width);
                int height = static_cast<int>(size.height);

                XcursorImage* cursorImage = XcursorImageCreate(width, height);

                if (!cursorImage)
                {
                    Log(Log::Level::ERR) << "Failed to create cursor image";
                    return false;
                }

                cursorImage->xhot = static_cast<int>(hotSpot.x);
                cursorImage->yhot = height - static_cast<int>(hotSpot.y) - 1;
                cursorImage->delay = 0;

                unsigned char* target = reinterpret_cast<unsigned char*>(cursorImage->pixels);

                for (int i = 0; i < width * height; ++i)
                {
                    // premultiply alpha
                    float opacity = data[i * 4 + 3] / 255.0f;

                    target[i * 4 + 0] = static_cast<unsigned char>(data[i * 4 + 2] * opacity);
                    target[i * 4 + 1] = static_cast<unsigned char>(data[i * 4 + 1] * opacity);
                    target[i * 4 + 2] = static_cast<unsigned char>(data[i * 4 + 0] * opacity);
                    target[i * 4 + 3] = data[i * 4 + 3];
                }

                cursor = XcursorImageLoadCursor(display, cursorImage);

                XcursorImageDestroy(cursorImage);
            }

            reactivate();

            return true;
        }
    } // namespace input
} // namespace ouzel
