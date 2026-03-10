#pragma once
#include "App/Time.hpp"
#include "ECS/Scene.hpp"
#include "Window/Window.hpp"
#include "Graphics/Gl/Renderer.hpp"
#include "Project/Project.hpp"
#include "Input/State.hpp"

namespace Core::App
{
	struct Context
	{
		Context(const Time& time, Window& window, const Input::State& input, entt::dispatcher& eventDispatcher, Project& project)
			: time(time), window(window), input(input), eventDispatcher(eventDispatcher), project(project) { }

		const Time& time;
		Window& window;
		const Input::State& input;
		entt::dispatcher& eventDispatcher;
		Project& project;
	};
}