#pragma once
#include "App/Client.hpp"
#include "Events/Keyboard.hpp"

namespace App
{
	class SceneViewerApp : public Core::App::Client
	{
	public:
		void Init(const Core::App::InitContext& context) override;
		void Update(const Core::App::Context& context) override;
		void Render(const Core::App::Context& context) override;
	private:
		void OnKeyPressed(const Core::Events::KeyPressed& event);

		Core::Window* m_Window = nullptr;
	};
}
