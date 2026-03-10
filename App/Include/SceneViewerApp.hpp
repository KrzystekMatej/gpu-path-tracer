#pragma once
#include "App/Client.hpp"
#include "Events/Keyboard.hpp"

namespace App
{
	class SceneViewerApp : public Core::App::Client
	{
	public:
		void RegisterUserScripts(Core::Scripts::Catalog& scriptCatalog) const override;
		void RegisterUserResolvers(Core::ECS::SceneResolverRegistry& resolverRegistry) const override;
		void RegisterEventHandlers(entt::dispatcher& dispatcher, Core::Window& window) override;
		void Update(const Core::App::Context& context) override;
		void Render(const Core::App::Context& context) override;
	private:
		void OnKeyPressed(const Core::Events::KeyPressed& event);

		Core::Window* m_Window = nullptr;
	};
}
