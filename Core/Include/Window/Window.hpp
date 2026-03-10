#pragma once
#include <expected>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <optional>
#include "Utils/Error/Error.hpp"
#include "Window/WindowAttributes.hpp"
#include "Window/GraphicsContext.hpp"
#include <entt/entt.hpp>
#include "Events/Router.hpp"



namespace Core
{
	class Window
	{
	public:
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&& other) noexcept;
		Window& operator=(Window&& other) noexcept;
		~Window();

		static std::expected<Window, Utils::Error> Create(WindowAttributes windowAttributes);

		uint32_t GetWidth() const { return m_Attributes.width; }
		uint32_t GetHeight() const { return m_Attributes.height; }
		const WindowAttributes& GetAttributes() const { return m_Attributes; }
		const GraphicsContext& GetContext() const { return m_GraphicsContext; }
		GLFWwindow* GetRawHandle() const { return m_Handle; }
		void InitCallbacks();
		void SetEventRouter(std::unique_ptr<Events::WindowEventRouter> eventRouter) { m_EventRouter = std::move(eventRouter); }
		void PollEvents() const;
        bool IsOpen() const;
        void SwapBuffers() const;
		void Close() const;
	private:
		Window(GLFWwindow* handle, WindowAttributes attributes, GraphicsContext context);

		friend void KeyCallback(GLFWwindow*, int, int, int, int);
		friend void FramebufferSizeCallback(GLFWwindow*, int, int);
		friend void MouseButtonCallback(GLFWwindow*, int, int, int);
		friend void CursorPositionCallback(GLFWwindow*, double, double);
		friend void WindowCloseCallback(GLFWwindow*);

		GLFWwindow* m_Handle = nullptr;
		WindowAttributes m_Attributes;
		GraphicsContext m_GraphicsContext;
		std::unique_ptr<Events::WindowEventRouter> m_EventRouter;
	};
}