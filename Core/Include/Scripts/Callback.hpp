#pragma once
#include "ECS/Context.hpp"

namespace Core::Scripts
{
	using Callback = void(*)(const ECS::Context& context);
}