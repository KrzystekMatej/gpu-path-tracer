#pragma once
#include <Glfw/glfw3.h>

namespace Core::Runtime
{
	class ImGuiBackend
	{
	public:
		void Init(GLFWwindow* handle);
		void BeginFrame();
		void Render();
		void Shutdown();
	};
}