#pragma once
#include "Input/Types.hpp"
#include <glm/glm.hpp>

namespace Core::Events
{
	struct MouseButtonPressed
    {
        Input::MouseButton button;
        Input::ModifierMask modifiers;
    };

    struct MouseButtonReleased
    {
        Input::MouseButton button;
        Input::ModifierMask modifiers;
    };

    struct CursorMoved
    {
        glm::vec2 position;
    };
}