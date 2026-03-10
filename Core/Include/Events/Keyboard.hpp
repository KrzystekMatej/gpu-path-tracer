#pragma once
#include "Input/Types.hpp"

namespace Core::Events
{
	struct KeyPressed
	{
		Input::KeyCode key;
		Input::ModifierMask modifiers;
		bool repeat;
	};

	struct KeyReleased
	{
		Input::KeyCode key;
		Input::ModifierMask modifiers;
	};
}