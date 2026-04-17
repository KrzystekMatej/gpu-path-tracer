#pragma once
#include <yaml-cpp/yaml.h>
#include <entt/entt.hpp>
#include <expected>
#include <Core/Utils/Error.hpp>
#include <Core/Assets/Manager.hpp>

namespace Core::Ecs::SceneNodes
{
	struct BuildContext
	{
		YAML::Node node;
		entt::entity entity;
		entt::entity parent;
	};

	class Builder
	{
	public:
		virtual std::expected<void, Utils::Error> Build(
			const BuildContext& context, 
			entt::registry& registry, 
			Assets::Manager& assetManager) const = 0;
	};
}