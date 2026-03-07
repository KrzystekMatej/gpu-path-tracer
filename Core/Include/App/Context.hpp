#pragma once
#include "App/Time.hpp"
#include "ECS/Scene.hpp"
#include "Window/Window.hpp"
#include "Graphics/Gl/Renderer.hpp"
#include "Project/Project.hpp"

namespace Core::App
{
	struct Context
	{
		Context(const Time& time, Window& window, Project& project)
			: time(time), window(window), project(project) { }

		const Time& time;
		Window& window;
		Project& project;
	};
}