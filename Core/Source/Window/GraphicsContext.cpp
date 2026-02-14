#include <glad/gl.h>
#include "Window/GraphicsContext.hpp"

namespace Core
{
	GraphicsContext::GraphicsContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle) {}

	std::expected<GraphicsContext, Error> GraphicsContext::Create(GLFWwindow* windowHandle)
	{
		if (!windowHandle)
			return std::unexpected(Error("Invalid window handle provided for graphics context creation!"));
		
		return GraphicsContext(windowHandle);
	}

	void GraphicsContext::MakeCurrent() const
	{
		glfwMakeContextCurrent(m_WindowHandle);
	}

	void GraphicsContext::Detach() const
	{
		glfwMakeContextCurrent(nullptr);
	}

	void GraphicsContext::SetSwapInterval(int interval) const
	{
		glfwSwapInterval(interval);
	}

	GLVersion GraphicsContext::GetVersion() const
	{
		const char* vendor   = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
		const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
		const char* glsl = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

		int major = 0;
		int minor = 0;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);

		return {
			major,
			minor,
			vendor ? vendor : "",
			renderer ? renderer : "",
			glsl ? glsl : ""
		};
	}	
}
