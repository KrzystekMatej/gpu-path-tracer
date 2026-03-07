#pragma once
#include "App/Time.hpp"
#include "ECS/Scene.hpp"

namespace Core::Scripts
{
	using Awake = void(*)(ECS::Scene& scene);
	using Update = void(*)(ECS::Scene& scene, const App::Time& time);
}