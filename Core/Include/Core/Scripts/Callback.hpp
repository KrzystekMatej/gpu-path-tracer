#pragma once
#include <Core/Ecs/Context.hpp>

namespace Core::Scripts
{
	using Callback = void(*)(const Ecs::Context& context);
}