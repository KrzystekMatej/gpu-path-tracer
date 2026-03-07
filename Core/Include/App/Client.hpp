#pragma once
#include <memory>
#include "Context.hpp"
#include "Events/Event.hpp"
#include "Scripts/Catalog.hpp"
#include "ECS/Resolvers/SceneResolverRegistry.hpp"

namespace Core::App
{
	class Client
	{
	public:
		virtual void RegisterScripts(Scripts::Catalog& scriptCatalog) = 0;
		virtual void RegisterSceneResolvers(ECS::SceneResolverRegistry& resolverRegistry) = 0;
		virtual void OnEvent(const Context& context, const Event& event) = 0;
		virtual void Update(const Context& context) = 0;
		virtual void Render(const Context& context) = 0;
	};
}