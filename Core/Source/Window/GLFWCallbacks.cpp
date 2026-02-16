#include "Window/GLFWCallbacks.hpp"
#include "Window/Window.hpp"
#include "Events/EventDispatcher.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseButtonEvent.hpp"
#include "Events/CursorPositionEvent.hpp"
#include "Events/FrameBufferSizeEvent.hpp"

namespace Core
{
    void SetGLFWCallbacks(GLFWwindow* windowHandle)
    {
        glfwSetKeyCallback(windowHandle, KeyCallback);
        glfwSetFramebufferSizeCallback(windowHandle, FramebufferSizeCallback);
        glfwSetMouseButtonCallback(windowHandle, MouseButtonCallback);
        glfwSetCursorPosCallback(windowHandle, CursorPositionCallback);
    }

    void KeyCallback(GLFWwindow* windowHandle, int key, int scancode, int action, int mods)
    {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
        KeyEvent event(key, scancode, action, mods);
		window->m_EventCallback(event);
    }

    void FramebufferSizeCallback(GLFWwindow* windowHandle, int width, int height)
    {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
        FrameBufferSizeEvent event(width, height);
		window->m_Attributes.width = width;
		window->m_Attributes.height = height;
		window->m_EventCallback(event);
    }

    void MouseButtonCallback(GLFWwindow* windowHandle, int button, int action, int mods)
    {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
        if (action == GLFW_PRESS || action == GLFW_RELEASE)
        {
            MouseButtonEvent event(button, action, mods);
			window->m_EventCallback(event);
        }
    }

    void CursorPositionCallback(GLFWwindow* windowHandle, double x, double y)
    {
        Window* window = static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
        CursorPositionEvent event(x, y);
		window->m_EventCallback(event);
    }
}
