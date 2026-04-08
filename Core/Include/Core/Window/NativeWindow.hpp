#pragma once
#include <expected>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <optional>	
#include <entt/entt.hpp>
#include <Core/External/Glm.hpp>
#include <Core/Utils/Error.hpp>
#include <Core/Window/Attributes.hpp>
#include <Core/Window/GraphicsContext.hpp>
#include <Core/Events/Router.hpp>
#include <Core/Graphics/Gl/RenderSurface.hpp>



namespace Core::Window
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

	class NativeWindow
	{
	public:
		NativeWindow(const NativeWindow&) = delete;
		NativeWindow& operator=(const NativeWindow&) = delete;
		NativeWindow(NativeWindow&& other) noexcept;
		NativeWindow& operator=(NativeWindow&& other) noexcept;
		~NativeWindow();
		void Destroy();

		static std::expected<NativeWindow, Utils::Error> Create(Attributes windowAttributes);
		static std::expected<void, Utils::Error> InitBackend();
		static void TerminateBackend();

		uint32_t GetWidth() const { return m_Attributes.width; }
		uint32_t GetHeight() const { return m_Attributes.height; }
		const Attributes& GetAttributes() const { return m_Attributes; }
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
		NativeWindow(GLFWwindow* handle, Attributes attributes, GraphicsContext context);

		friend class GlfwCallbacks;

		GLFWwindow* m_Handle = nullptr;
		Attributes m_Attributes;
		GraphicsContext m_GraphicsContext;
		std::unique_ptr<Events::WindowEventRouter> m_EventRouter;
	};
}