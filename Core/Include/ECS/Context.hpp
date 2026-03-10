#pragma once
#include "App/Time.hpp"
#include "ECS/Scene.hpp"
#include "Window/Window.hpp"
#include "Project/Project.hpp"

namespace Core::ECS
{
	struct Context
	{
		Context(const App::Time& time, Scene& scene, Window& window)
			: time(time), scene(scene), window(window) {}

		const App::Time& time;
		Scene& scene;
		Window& window;
	};
}
