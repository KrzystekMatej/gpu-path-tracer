#pragma once
#include <Core/Runtime/ImGuiClient.hpp>
#include <Core/Events/Keyboard.hpp>
#include <Core/Graphics/Gl/RenderTarget.hpp>

namespace App
{
	class SceneViewerApp : public Core::Runtime::ImGuiClient
	{
	public:
		void ImGuiInit(const Core::Runtime::InitContext& context) override;
		void Update(const Core::Runtime::Context& context) override;
		void ImGuiBuild(const Core::Runtime::Context& context) override;
		void ImGuiShutdown() override;
	private:
		void OnKeyPressed(const Core::Events::KeyPressed& event);

		Core::Window::NativeWindow* m_Window = nullptr;
		std::unique_ptr<Core::Graphics::Gl::RenderTarget> m_SceneTarget;

		bool m_LockDisplayRatio = true;
		bool m_ShowOverlay = true;
		int m_SelectedMode = 0;
		float m_ClearStrength = 1.0f;
	};
}
