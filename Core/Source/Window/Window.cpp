#include "Window/Window.hpp"
#include "Window/GLFWCallbacks.hpp"
#include <spdlog/spdlog.h>

namespace Core
{
	namespace
	{
		int ToGlfwCursorMode(CursorMode mode)
		{
			switch (mode)
			{
				case CursorMode::Normal: return GLFW_CURSOR_NORMAL;
				case CursorMode::Hidden: return GLFW_CURSOR_HIDDEN;
				case CursorMode::Disabled: return GLFW_CURSOR_DISABLED;
			}

			return GLFW_CURSOR_NORMAL;
		}

		CursorMode FromGlfwCursorMode(int mode)
		{
			switch (mode)
			{
				case GLFW_CURSOR_NORMAL: return CursorMode::Normal;
				case GLFW_CURSOR_HIDDEN: return CursorMode::Hidden;
				case GLFW_CURSOR_DISABLED: return CursorMode::Disabled;
			}

			return CursorMode::Normal;
		}
	}

	Window::Window(GLFWwindow* handle, WindowAttributes attributes, GraphicsContext context)
		: m_Handle(handle), m_Attributes(std::move(attributes)), m_GraphicsContext(std::move(context)) {}

	Window::Window(Window&& other) noexcept
		: m_Handle(std::exchange(other.m_Handle, nullptr)),
		  m_Attributes(std::move(other.m_Attributes)),
		  m_GraphicsContext(std::move(other.m_GraphicsContext)),
		  m_EventRouter(std::move(other.m_EventRouter))
	{
		if (m_Handle)
			glfwSetWindowUserPointer(m_Handle, this);
	}

	Window& Window::operator=(Window&& other) noexcept
	{
		if (this != &other)
		{
			if (m_Handle)
				glfwDestroyWindow(m_Handle);

			m_Handle = std::exchange(other.m_Handle, nullptr);
			m_Attributes = std::move(other.m_Attributes);
			m_GraphicsContext = std::move(other.m_GraphicsContext);
			m_EventRouter = std::move(other.m_EventRouter);

			if (m_Handle)
				glfwSetWindowUserPointer(m_Handle, this);
		}
		return *this;
	}

	Window::~Window()
	{
		if (m_Handle)
			glfwDestroyWindow(m_Handle);
	}

	std::expected<Window, Utils::Error> Window::Create(WindowAttributes windowAttributes)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 8);

		GLFWwindow* windowHandle = glfwCreateWindow(windowAttributes.width, windowAttributes.height, windowAttributes.title.c_str(), nullptr, nullptr);

		if (!windowHandle) 
			return std::unexpected(Utils::Error("Failed to create GLFW window handle!"));
		glfwSetWindowSizeLimits(windowHandle, windowAttributes.minWidth, windowAttributes.minHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);
		glfwSetInputMode(windowHandle, GLFW_LOCK_KEY_MODS, GLFW_TRUE);

		auto graphicsContext = GraphicsContext::Create(windowHandle);

		if (!graphicsContext)
			return std::unexpected(graphicsContext.error());

		return Window(windowHandle, std::move(windowAttributes), std::move(graphicsContext).value());
	}

	std::expected<void, Utils::Error> Window::InitBackend()
	{
		if (!glfwInit())
			return std::unexpected(Utils::Error("Failed to initialize GLFW!"));
		glfwSetErrorCallback(GlfwCallbacks::Error);
		return {};
	}

	void Window::TerminateBackend()
	{
		glfwTerminate();
	}

	GlfwVersion Window::GetVersion() const
	{
		int major, minor, revision;
		glfwGetVersion(&major, &minor, &revision);
		return { major, minor, revision };
	}

	void Window::InitCallbacks()
	{
		glfwSetWindowUserPointer(m_Handle, this);
		GlfwCallbacks::SetAll(m_Handle);
	}

	void Window::PollEvents() const
    {
        glfwPollEvents();
    }

    bool Window::IsOpen() const
    {
        return !glfwWindowShouldClose(m_Handle);
    }

	void Window::Close() const
	{
		glfwSetWindowShouldClose(m_Handle, GLFW_TRUE);
	}

    void Window::SwapBuffers() const
    {
        glfwSwapBuffers(m_Handle);
    }

	glm::vec2 Window::GetCursorPosition() const
	{
		double x, y;
		glfwGetCursorPos(m_Handle, &x, &y);
		return glm::vec2(static_cast<float>(x), static_cast<float>(y));
	}

	void Window::SetCursorPosition(glm::vec2 position) const
	{
		glfwSetCursorPos(m_Handle, position.x, position.y);
	}

	void Window::SetCursorMode(CursorMode mode) const
	{
		glfwSetInputMode(m_Handle, GLFW_CURSOR, ToGlfwCursorMode(mode));
	}

	CursorMode Window::GetCursorMode() const
	{
		return FromGlfwCursorMode(glfwGetInputMode(m_Handle, GLFW_CURSOR));
	}

	bool Window::SupportsRawMouseMotion() const
	{
		return glfwRawMouseMotionSupported() == GLFW_TRUE;
	}

	void Window::SetRawMouseMotionEnabled(bool enabled) const
	{
		if (!SupportsRawMouseMotion())
			return;

		glfwSetInputMode(m_Handle, GLFW_RAW_MOUSE_MOTION, enabled ? GLFW_TRUE : GLFW_FALSE);
	}

	bool Window::IsRawMouseMotionEnabled() const
	{
		if (!SupportsRawMouseMotion())
			return false;

		return glfwGetInputMode(m_Handle, GLFW_RAW_MOUSE_MOTION) == GLFW_TRUE;
	}
}