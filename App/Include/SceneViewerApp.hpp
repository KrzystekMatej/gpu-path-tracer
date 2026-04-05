#pragma once
#include "App/ImGuiClient.hpp"
#include "Events/Keyboard.hpp"
#include "Graphics/Gl/RenderTarget.hpp"

namespace App
{
	class SceneViewerApp : public Core::App::ImGuiClient
	{
	public:
		void ImGuiInit(const Core::App::InitContext& context) override;
		void Update(const Core::App::Context& context) override;
		void ImGuiBuild(const Core::App::Context& context) override;
		void ImGuiShutdown() override;
	private:
		void OnKeyPressed(const Core::Events::KeyPressed& event);

		Core::Window* m_Window = nullptr;
		std::unique_ptr<Core::Graphics::Gl::RenderTarget> m_SceneTarget;

		bool m_LockDisplayRatio = true;
		bool m_ShowOverlay = true;
		int m_SelectedMode = 0;
		float m_ClearStrength = 1.0f;
	};
}
