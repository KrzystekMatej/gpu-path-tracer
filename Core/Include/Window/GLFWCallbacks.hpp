#pragma once
#include <GLFW/glfw3.h>

namespace Core
{
	class GlfwCallbacks
	{
	public:
		static void SetAll(GLFWwindow* windowHandle);
		static void Key(GLFWwindow* windowHandle, int key, int scancode, int action, int mods);
		static void MouseButton(GLFWwindow* windowHandle, int button, int action, int mods);
		static void CursorPosition(GLFWwindow* windowHandle, double x, double y);
		static void WindowClose(GLFWwindow* windowHandle);
		static void FramebufferSize(GLFWwindow* windowHandle, int width, int height);
		static void Error(int error, const char* description);
	};
}
