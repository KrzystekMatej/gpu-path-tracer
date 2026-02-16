#pragma once
#include "Window/Window.hpp"
#include "Render/Renderer.hpp"
#include "Project/Project.hpp"

namespace Core
{
	struct AppContext
	{
		AppContext(Window& windowRef, Renderer& rendererRef, Project& projectRef)
			: window(windowRef), renderer(rendererRef), project(projectRef) {}

		Window& window;
		Renderer& renderer;
		Project& project;
	};
}