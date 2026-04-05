#include <glad/gl.h>
#include "Window/GraphicsContext.hpp"
#include <spdlog/spdlog.h>

namespace Core
{
	namespace
	{
		void APIENTRY DebugCallback(
			GLenum source,
			GLenum type,
			GLuint id,
			GLenum severity,
			GLsizei length,
			const GLchar* message,
			const void* userParam)
		{
			if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
				return;

			spdlog::level::level_enum level;

			switch (severity)
			{
				case GL_DEBUG_SEVERITY_HIGH:         
					level = spdlog::level::err; 
					break;
				case GL_DEBUG_SEVERITY_MEDIUM:       
					level = spdlog::level::warn; 
					break;
				case GL_DEBUG_SEVERITY_LOW:          
					level = spdlog::level::info; 
					break;
				default:                             
					level = spdlog::level::debug; 
					break;
			}

			spdlog::log(level,
				"[OpenGL] id={} source=0x{:X} type=0x{:X} msg={}",
				id, source, type, message);
		}
	}

	GraphicsContext::GraphicsContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle) {}

	std::expected<GraphicsContext, Utils::Error> GraphicsContext::Create(GLFWwindow* windowHandle)
	{
		assert(windowHandle != nullptr);
		GraphicsContext context(windowHandle);
		context.MakeCurrent();

		if (auto loadResult = LoadOpenGL(); !loadResult)
			return std::unexpected(std::move(loadResult).error());

		context.InitDebugCallback();
		return context;
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

	std::expected<void, Utils::Error> GraphicsContext::LoadOpenGL()
	{
		static bool loaded = false;

		if (!loaded)
		{
			if (!gladLoadGL(glfwGetProcAddress))
				return std::unexpected(Utils::Error("Failed to initialize GLAD."));

			loaded = true;
		}

		return {};
	}

	void GraphicsContext::InitDebugCallback()
	{
	    GLint flags = 0;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

		if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			glEnable(GL_DEBUG_OUTPUT);
#ifndef NDEBUG
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
			glDebugMessageCallback(DebugCallback, nullptr);

			glDebugMessageControl(
				GL_DONT_CARE,
				GL_DONT_CARE,
				GL_DEBUG_SEVERITY_NOTIFICATION,
				0,
				nullptr,
				GL_FALSE);
		}
	}
}
