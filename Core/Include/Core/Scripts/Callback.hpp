#pragma once
#include <Core/ECS/Context.hpp>

namespace Core::Scripts
{
	using Callback = void(*)(const ECS::Context& context);
}