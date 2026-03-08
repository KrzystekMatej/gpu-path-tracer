#pragma once
#include "ECS/Resolvers/Resolver.hpp"

namespace Core::ECS
{
	class ModelResolver : public SceneNodeResolver
	{
	public:
		virtual std::expected<void, Utils::Error> Resolve(
			const SceneNodeContext& context,
			entt::registry& registry,
			Assets::Manager& assetManager) const override;
	};
}
