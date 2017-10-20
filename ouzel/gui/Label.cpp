// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "core/Setup.h"
#include "Label.hpp"
#include "core/Engine.hpp"
#include "graphics/Renderer.hpp"
#include "scene/Layer.hpp"
#include "scene/Camera.hpp"
#include "BMFont.hpp"
#include "assets/Cache.hpp"

namespace ouzel
{
    namespace gui
    {
        Label::Label(const std::string& aText,
                     const std::string& fontFile,
                     float fontSize,
                     Color color,
                     const Vector2& textAnchor):
            text(aText),
            labelDrawable(std::make_shared<scene::TextRenderer>(fontFile, true, fontSize, text, color, textAnchor))
        {
            addComponent(labelDrawable.get());
            labelDrawable->setText(text);

            pickable = true;
        }

        void Label::setText(const std::string& newText)
        {
            text = newText;
            labelDrawable->setText(text);
        }
    } // namespace gui
} // namespace ouzel
