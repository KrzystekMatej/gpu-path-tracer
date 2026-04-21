#include <Core/Ecs/Scene.hpp>
#include <spdlog/spdlog.h>
#include <stack>
#include <Core/Utils/Yaml.hpp>
#include <Core/Graphics/Ecs/Camera.hpp>
#include <Core/Ecs/Transform.hpp>
#include <Core/Ecs/Hierarchy.hpp>

namespace Core::Ecs
{
	std::expected<Scene, Utils::Error> Scene::Create(
		Import::Scene scene,
		const BuilderRegistry& builderRegistry,
		Assets::Manager& assetManager)
	{
		entt::registry registry;
		std::stack<BuildContext> nodes;

		for (const auto& rootNode : scene.sceneRoot)
		{
			nodes.push(BuildContext{
				.node = rootNode,
				.entity = registry.create(),
				.parent = entt::null
			});
		}

		while (!nodes.empty())
		{
			BuildContext current = nodes.top();
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
					Utils::Guid builderId = Utils::Hasher::MakeId(typeStr);
					
					auto builderResult = builderRegistry.Get(builderId);
					if (!builderResult)
						return std::unexpected(Utils::Error(builderResult.error().Message()));

					const Builder& builder = builderResult.value();

					BuildContext context{
						.node = componentNode,
						.entity = current.entity,
						.parent = current.parent
					};

					auto ok = builder.Build(context, registry, assetManager);

					if (!ok)
						return std::unexpected(ok.error());
				}
			}

			YAML::Node childNodes = current.node["children"];

			if (childNodes)
			{
				if (!childNodes.IsSequence())
					return std::unexpected(Utils::Error("Entity 'children' must be a sequence"));

				std::vector<entt::entity> childEntities;
				childEntities.reserve(childNodes.size());

				for (const auto& childNode : childNodes)
				{
					entt::entity childEntity = registry.create();
					registry.emplace<Parent>(childEntity, current.entity);

					childEntities.emplace_back(childEntity);
					nodes.push(BuildContext{
						.node = childNode,
						.entity = childEntity,
						.parent = current.entity
					});
				}

				if (!childEntities.empty())
					registry.emplace<Children>(current.entity, std::move(childEntities));
			}
		}

		auto view = registry.view<Graphics::Ecs::Camera>();

		if (view.size() == 1)
		{
			entt::entity cameraEntity = *view.begin();
			if (!registry.all_of<Transform, WorldTransform>(cameraEntity))
			{
				return std::unexpected(Utils::Error("Camera entity must have a Transform component"));
			}

			return Scene(std::move(registry), cameraEntity, SceneState::Empty, std::move(scene.scripts));
		}
		
		return std::unexpected(Utils::Error("Scene must contain exactly one camera!"));
	}
}