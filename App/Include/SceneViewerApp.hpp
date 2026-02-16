#pragma once
#include "App/AppClient.hpp"
#include "Events/Event.hpp"

namespace App
{
	class SceneViewerApp : public Core::AppClient
	{
	public:
		void OnEvent(const Core::AppContext& context, const Core::Event& event) override;
		void Update(const Core::AppContext& context) override;
		void Render(const Core::AppContext& context) override;
	};
}
