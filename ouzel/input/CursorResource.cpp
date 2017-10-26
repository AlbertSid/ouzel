// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "CursorResource.hpp"
#include "input/Input.hpp"
#include "core/Engine.hpp"

namespace ouzel
{
    namespace input
    {
        CursorResource::CursorResource()
        {
        }

        CursorResource::~CursorResource()
        {
        }

        bool CursorResource::init(SystemCursor newSystemCursor)
        {
            systemCursor = newSystemCursor;

            return true;
        }

        bool CursorResource::init(const std::vector<uint8_t>& newData,
                                  const Size2& newSize,
                                  graphics::PixelFormat newPixelFormat,
                                  const Vector2& newHotSpot)
        {
            data = newData;
            size = newSize;
            pixelFormat = newPixelFormat;
            hotSpot = newHotSpot;

            return true;
        }


        void CursorResource::reactivate()
        {
            if (engine->getInput()->currentCursorResource == this)
            {
                engine->getInput()->activateCursorResource(this);
            }
        }
    } // namespace input
} // namespace ouzel
