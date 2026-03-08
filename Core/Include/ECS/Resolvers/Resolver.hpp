#pragma once
#include <yaml-cpp/yaml.h>
#include <entt/entt.hpp>
#include <expected>
#include "Utils/Error/Error.hpp"
#include "Assets/Manager.hpp"

namespace Core::ECS
{
	struct SceneNodeContext
	{
		YAML::Node node;
		entt::entity entity;
		entt::entity parent;
	};

	class SceneNodeResolver
	{
	public:
		virtual std::expected<void, Utils::Error> Resolve(
			const SceneNodeContext& context, 
			entt::registry& registry, 
			Assets::Manager& assetManager) const = 0;
	};
}