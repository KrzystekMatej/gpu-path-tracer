#pragma once
#include "App/Time.hpp"
#include "ECS/Scene.hpp"
#include "Window/Window.hpp"
#include "Project/Project.hpp"
#include "Input/State.hpp"	

namespace Core::ECS
{
	struct Context
	{
		Context(const App::Time& time, Window& window, const Input::State& input, Scene& scene)
			: time(time), window(window), input(input), scene(scene) { }

		const App::Time& time;
		Window& window;
		const Input::State& input;
		Scene& scene;
	};
}
