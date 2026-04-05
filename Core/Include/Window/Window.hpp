#pragma once
#include <expected>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <optional>
#include "External/Glm.hpp"
#include "Utils/Error/Error.hpp"
#include "Window/WindowAttributes.hpp"
#include "Window/GraphicsContext.hpp"
#include <entt/entt.hpp>
#include "Events/Router.hpp"
#include "Graphics/Gl/RenderSurface.hpp"



namespace Core
{
	enum class CursorMode
	{
		Normal,
		Hidden,
		Disabled
	};

	struct GlfwVersion
	{
		int major;
		int minor;
		int revision;
	};

	class Window
	{
	public:
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&& other) noexcept;
		Window& operator=(Window&& other) noexcept;
		~Window();

		static std::expected<Window, Utils::Error> Create(WindowAttributes windowAttributes);
		static std::expected<void, Utils::Error> InitBackend();
		static void TerminateBackend();

		uint32_t GetWidth() const { return m_Attributes.width; }
		uint32_t GetHeight() const { return m_Attributes.height; }
		const WindowAttributes& GetAttributes() const { return m_Attributes; }
		GlfwVersion GetVersion() const;
		const GraphicsContext& GetContext() const { return m_GraphicsContext; }
		GLFWwindow* GetRawHandle() const { return m_Handle; }
		Graphics::Gl::RenderSurface GetRenderSurface() const { return Graphics::Gl::RenderSurface(0, GetWidth(), GetHeight()); }

		void InitCallbacks();
		void SetEventRouter(std::unique_ptr<Events::WindowEventRouter> eventRouter) { m_EventRouter = std::move(eventRouter); }

		void PollEvents() const;
        bool IsOpen() const;
        void SwapBuffers() const;
		void Close() const;

		glm::vec2 GetCursorPosition() const;
		void SetCursorPosition(glm::vec2 position) const;

		void SetCursorMode(CursorMode mode) const;
		CursorMode GetCursorMode() const;

		bool SupportsRawMouseMotion() const;
		void SetRawMouseMotionEnabled(bool enabled) const;
		bool IsRawMouseMotionEnabled() const;
	private:
		Window(GLFWwindow* handle, WindowAttributes attributes, GraphicsContext context);

		friend class GlfwCallbacks;

		GLFWwindow* m_Handle = nullptr;
		WindowAttributes m_Attributes;
		GraphicsContext m_GraphicsContext;
		std::unique_ptr<Events::WindowEventRouter> m_EventRouter;
	};
}