#pragma once
#include <GLFW/glfw3.h>

namespace Core
{
	void SetGLFWCallbacks(GLFWwindow* windowHandle);
	void KeyCallback(GLFWwindow* windowHandle, int key, int scancode, int action, int mods);
	void FramebufferSizeCallback(GLFWwindow* windowHandle, int width, int height);
	void MouseButtonCallback(GLFWwindow* windowHandle, int button, int action, int mods);
	void CursorPositionCallback(GLFWwindow* windowHandle, double x, double y);
}
