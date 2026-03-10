#include "ECS/Scene.hpp"
#include <spdlog/spdlog.h>
#include <stack>
#include "ECS/Components/Camera.hpp"
#include "Utils/Yaml.hpp"
#include "ECS/Components/Transform.hpp"
#include "ECS/Components/Hierarchy.hpp"

namespace Core::ECS
{
	std::expected<Scene, Utils::Error> Scene::Create(
		IO::Scene scene,
		const ECS::SceneResolverRegistry& resolverRegistry,
		Assets::Manager& assetManager)
	{
		entt::registry registry;
		std::stack<SceneNodeContext> nodes;

		for (const auto& rootNode : scene.sceneRoot)
		{
			nodes.push(SceneNodeContext{
				.node = rootNode,
				.entity = registry.create(),
				.parent = entt::null
			});
		}

		while (!nodes.empty())
		{
			SceneNodeContext current = nodes.top();
			nodes.pop();

			YAML::Node components = current.node["components"];

			if (!components)
			{
				spdlog::warn("Entity with id '{}' has no components.", entt::to_integral(current.entity));
			}
			else
			{
				if (!components.IsSequence())
					return std::unexpected(Utils::Error("Entity 'components' must be a sequence"));

				for (const auto& componentNode : components)
				{
					auto typeResult = Utils::Yaml::GetString(componentNode, "type");

					if (!typeResult)
						return std::unexpected(Utils::Error(std::move(typeResult).error()));

					std::string typeStr = std::move(typeResult).value();
					Utils::Guid resolverId = Utils::Hasher::MakeId(typeStr);
					
					auto resolverResult = resolverRegistry.GetResolver(resolverId);
					if (!resolverResult)
						return std::unexpected(Utils::Error(resolverResult.error().Message()));

					const SceneNodeResolver& resolver = resolverResult.value();

					SceneNodeContext context{
						.node = componentNode,
						.entity = current.entity,
						.parent = current.parent
					};

					auto ok = resolver.Resolve(context, registry, assetManager);

					if (!ok)
						return std::unexpected(ok.error());
				}
			}

			YAML::Node children = current.node["children"];

			if (children)
			{
				if (!children.IsSequence())
					return std::unexpected(Utils::Error("Entity 'children' must be a sequence"));

				for (const auto& childNode : children)
				{
					entt::entity childEntity = registry.create();
					registry.emplace<Components::Parent>(childEntity, current.entity);
					nodes.push(SceneNodeContext{
						.node = childNode,
						.entity = childEntity,
						.parent = current.entity
					});
				}
			}
		}

		auto view = registry.view<Components::Camera>();

		if (view.size() == 1)
		{
			entt::entity cameraEntity = *view.begin();
			return Scene(std::move(registry), cameraEntity, SceneState::Empty, std::move(scene.scripts));
		}
		
		return std::unexpected(Utils::Error("Scene must contain exactly one camera!"));
	}
}