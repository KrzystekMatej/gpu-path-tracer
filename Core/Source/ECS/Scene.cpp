#include "ECS/Scene.hpp"
#include <spdlog/spdlog.h>
#include <stack>

namespace Core::ECS
{
	namespace
	{
		struct NodeContext
		{
			YAML::Node node;
			entt::entity entity;
		};
	}


	std::expected<Scene, Utils::Error> Scene::Create(IO::Scene scene, const ECS::SceneResolverRegistry& resolverRegistry)
	{
		entt::registry registry;
		entt::entity camera = entt::null;

		std::stack<NodeContext> nodes;
		NodeContext current{ scene.sceneRoot, entt::null };

		while (!nodes.empty())
		{
			for (const auto& node : current.node)
			{
				current.entity = registry.create();
				if (!node["components"])
					spdlog::warn("Entity with id '{}' has no components.", entt::to_integral(current.entity));
					continue;

				YAML::Node components = node["components"];

				for (const auto& component : components)
				{
					if (!component["type"])
						return std::unexpected(Utils::Error("Component is missing 'type' field"));

					std::string typeStr = component["type"].as<std::string>();
					Utils::Guid resolverId = Utils::Hasher::MakeId(typeStr);
					auto resolverResult = resolverRegistry.GetResolver(resolverId);
					if (!resolverResult)
						return std::unexpected(Utils::Error(resolverResult.error().Message()));

					const SceneNodeResolver& resolver = resolverResult.value();
					YAML::Node componentNode = component;

					resolver.Resolve(componentNode, current.entity, registry);
				}

				if (node["children"])
					continue;

				YAML::Node children = node["children"];

				for (const auto& child : children)
				{
					nodes.emplace(child, registry.create());
				}
			}
			current = nodes.top();
			nodes.pop();
		}
		return Scene(std::move(registry), camera, SceneState::Empty, std::move(scene.scripts));
	}
}