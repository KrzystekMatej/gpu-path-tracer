#pragma once
#include <expected>
#include <glfw/glfw3.h>
#include <glm/vec2.hpp>
#include <memory>
#include <string>
#include "Error/Error.hpp"
#include "Window/WindowAttributes.hpp"


namespace Core
{
	class Window
	{
	public:
		static std::expected<Window, Error> Create(WindowAttributes windowAttributes);

		uint32_t GetWidth() const { return m_Attributes.Width; }
		uint32_t GetHeight() const { return m_Attributes.Height; }
		const WindowAttributes& GetAttributes() const { return m_Attributes; }

		void PollEvents() const;
        bool IsOpen() const;
        void Clear() const;
        void SwapBuffers() const;
	private:
		Window(GLFWwindow* handle, WindowAttributes attributes);

		GLFWwindow* m_Handle;
		WindowAttributes m_Attributes;
	};
}