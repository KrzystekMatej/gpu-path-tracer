#pragma once
#include "App/Client.hpp"
#include "Events/Event.hpp"

namespace App
{
	class SceneViewerApp : public Core::App::Client
	{
	public:
		void RegisterUserScripts(Core::Scripts::Catalog& scriptCatalog) override;
		void RegisterUserResolvers(Core::ECS::SceneResolverRegistry& resolverRegistry) override;
		void OnEvent(const Core::App::Context& context, const Core::Event& event) override;
		void Update(const Core::App::Context& context) override;
		void Render(const Core::App::Context& context) override;
	};
}
