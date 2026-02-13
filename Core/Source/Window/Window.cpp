#include <glad/gl.h>
#include "Window/Window.hpp"

namespace Core
{
	Window::Window(GLFWwindow* handle, WindowAttributes attributes)
		: m_Handle(handle), m_Attributes(std::move(attributes)) {}

	std::expected<Window, Error> Window::Create(WindowAttributes windowAttributes)
	{
		if (!glfwInit())
			return std::unexpected(Error("Failed to initialize GLFW!"));

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
		glfwMakeContextCurrent(windowHandle);
		glfwSwapInterval(1);

		if (!gladLoadGL(glfwGetProcAddress))
			return std::unexpected(Error("Failed to initialize GLAD!"));

		if (!glfwExtensionSupported("GL_ARB_bindless_texture")) 
			return std::unexpected(Error(("GL_ARB_bindless_texture not supported!")));


		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		return Window(windowHandle, std::move(windowAttributes));
	}

	void Window::PollEvents() const
    {
        glfwPollEvents();
    }

    bool Window::IsOpen() const
    {
        return !glfwWindowShouldClose(m_Handle);
    }

    void Window::Clear() const
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void Window::SwapBuffers() const
    {
        glfwSwapBuffers(m_Handle);
    }
}