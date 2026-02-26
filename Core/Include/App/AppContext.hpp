#pragma once
#include "Window/Window.hpp"
#include "Graphics/Gl/Renderer.hpp"
#include "Project/Project.hpp"

namespace Core
{
	struct AppContext
	{
		AppContext(Window& windowRef, Graphics::Gl::Renderer& rendererRef, Project& projectRef)
			: window(windowRef), renderer(rendererRef), project(projectRef) {}

		Window& window;
		Graphics::Gl::Renderer& renderer;
		Project& project;
	};
}