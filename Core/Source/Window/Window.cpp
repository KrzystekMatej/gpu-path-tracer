#include <glad/gl.h>
#include "Window/Window.hpp"

namespace Core
{
	Window::Window(GLFWwindow* handle, WindowAttributes attributes, GraphicsContext context)
		: m_Handle(handle), m_Attributes(std::move(attributes)), m_GraphicsContext(std::move(context)) {}

	std::expected<Window, Error> Window::Create(WindowAttributes windowAttributes)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 8);

		GLFWwindow* windowHandle = glfwCreateWindow(windowAttributes.Width, windowAttributes.Height, windowAttributes.Title.c_str(), nullptr, nullptr);

		if (!windowHandle) 
			return std::unexpected(Error("Failed to create GLFW window handle!"));
		glfwSetWindowSizeLimits(windowHandle, windowAttributes.MinWidth, windowAttributes.MinHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);

		GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);

		if (!cursor) 
			return std::unexpected(Error("Failed to create cursor, using default system cursor."));

		glfwSetCursor(windowHandle, cursor);

		auto graphicsContext = GraphicsContext::Create(windowHandle);

		if (!graphicsContext)
			return std::unexpected(graphicsContext.error());

		return Window(windowHandle, std::move(windowAttributes), std::move(graphicsContext).value());
	}

	void Window::PollEvents() const
    {
        glfwPollEvents();
    }

    bool Window::IsOpen() const
    {
        return !glfwWindowShouldClose(m_Handle);
    }

    void Window::SwapBuffers() const
    {
        glfwSwapBuffers(m_Handle);
    }
}