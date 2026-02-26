#pragma once
#include <expected>
#include <glfw/glfw3.h>
#include <memory>
#include <string>
#include "Error/Error.hpp"
#include "Window/WindowAttributes.hpp"
#include "Window/GraphicsContext.hpp"
#include "Events/Event.hpp"




namespace Core
{
	class Window
	{
	public:
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) noexcept = default;
		Window& operator=(Window&&) noexcept = default;

		static std::expected<Window, Error> Create(WindowAttributes windowAttributes);

		uint32_t GetWidth() const { return m_Attributes.width; }
		uint32_t GetHeight() const { return m_Attributes.height; }
		const WindowAttributes& GetAttributes() const { return m_Attributes; }
		const GraphicsContext& GetContext() const { return m_GraphicsContext; }
		GLFWwindow* GetRawHandle() const { return m_Handle; }
		void InitCallbacks();
		void SetEventCallback(EventCallback callback) { m_EventCallback = std::move(callback); }
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

		GLFWwindow* m_Handle;
		WindowAttributes m_Attributes;
		GraphicsContext m_GraphicsContext;
		EventCallback m_EventCallback;
	};
}