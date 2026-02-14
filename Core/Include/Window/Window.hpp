#pragma once
#include <expected>
#include <glfw/glfw3.h>
#include <glm/vec2.hpp>
#include <memory>
#include <string>
#include "Error/Error.hpp"
#include "Window/WindowAttributes.hpp"
#include "Window/GraphicsContext.hpp"


namespace Core
{
	class Window
	{
	public:
		static std::expected<Window, Error> Create(WindowAttributes windowAttributes);

		uint32_t GetWidth() const { return m_Attributes.Width; }
		uint32_t GetHeight() const { return m_Attributes.Height; }
		const WindowAttributes& GetAttributes() const { return m_Attributes; }
		const GraphicsContext& GetContext() const { return m_GraphicsContext; }
		const GraphicsContext* GetContextPtr() const { return &m_GraphicsContext; }
		GLFWwindow* GetRawHandle() const { return m_Handle; }


		void PollEvents() const;
        bool IsOpen() const;
        void SwapBuffers() const;
	private:
		Window(GLFWwindow* handle, WindowAttributes attributes, GraphicsContext context);

		GLFWwindow* m_Handle;
		WindowAttributes m_Attributes;
		GraphicsContext m_GraphicsContext;
	};
}