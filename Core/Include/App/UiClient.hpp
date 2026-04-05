#pragma once
#include <memory>
#include "External/Glm.hpp"
#include "Context.hpp"
#include "Scripts/Catalog.hpp"
#include "ECS/Resolvers/SceneResolverRegistry.hpp"
#include "ECS/Systems/Render.hpp"
#include "Graphics/Gl/RenderTarget.hpp"

namespace Core::App
{
	class UiClient
	{
	public:
		virtual ~UiClient() = default;
		virtual void Init(const InitContext& context) = 0;
		virtual void Update(const Context& context) = 0;
		virtual void BuildUi(const Context& context) = 0;
		virtual void CommitUi() = 0;
		virtual void Shutdown() = 0;
	};
}