#pragma once
#include <glm/glm.hpp>
#include <Core/Input/Types.hpp>

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