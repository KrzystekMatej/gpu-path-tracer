#pragma once
#include <yaml-cpp/yaml.h>
#include <entt/entt.hpp>

namespace Core::ECS
{
	class SceneNodeResolver
	{
	public:
		virtual void Resolve(YAML::Node& currentNode, entt::entity currentEntity, entt::registry& registry) const = 0;
	};
}