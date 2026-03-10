#pragma once
#include <memory>
#include "Context.hpp"
#include "Scripts/Catalog.hpp"
#include "ECS/Resolvers/SceneResolverRegistry.hpp"

namespace Core::App
{
	class Client
	{
	public:
		virtual void RegisterUserScripts(Scripts::Catalog& scriptCatalog) const = 0;
		virtual void RegisterUserResolvers(ECS::SceneResolverRegistry& resolverRegistry) const = 0;
		virtual void RegisterEventHandlers(entt::dispatcher& dispatcher, Window& window) = 0;
		virtual void Update(const Context& context) = 0;
		virtual void Render(const Context& context) = 0;
	};
}