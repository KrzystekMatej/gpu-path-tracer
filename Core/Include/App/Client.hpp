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
		virtual void Init(const InitContext& context) = 0;
		virtual void Update(const Context& context) = 0;
		virtual void Render(const Context& context) = 0;
	};
}