#include "ECS/Resolvers/Model.hpp"
#include "ECS/Components/Transform.hpp"
#include "Utils/Yaml.hpp"
#include "ECS/Components/Graphics.hpp"
#include "ECS/Components/Hierarchy.hpp"

namespace Core::ECS
{
	std::expected<void, Utils::Error> ModelResolver::Resolve(
		const SceneNodeContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		auto pathResult = Utils::Yaml::GetString(context.node, "path");
		if (!pathResult)
			return std::unexpected(Utils::Error(std::move(pathResult).error()));

		auto modelHandleResult = assetManager.ImportObj(std::move(pathResult).value());
		if (!modelHandleResult)
			return std::unexpected(Utils::Error(std::move(modelHandleResult).error()));

		auto modelResult = assetManager.GetStorage().Get(modelHandleResult.value());
		const Assets::Model& model = modelResult.value();
		std::vector<entt::entity> children;
		children.reserve(model.parts.size());

		for (const auto& part : model.parts)
		{
			const entt::entity child = registry.create();

			children.emplace_back(child);
			registry.emplace<Components::Parent>(child, context.entity);
			registry.emplace<Components::Transform>(
				child,
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::identity<glm::quat>(),
				glm::vec3(1.0f, 1.0f, 1.0f));
			registry.emplace<Components::WorldTransform>(child);
			registry.emplace<Components::Mesh>(child, part.mesh);
			registry.emplace<Components::Material>(child, part.material);
		}

		if (!children.empty())
			registry.emplace<Components::Children>(context.entity, std::move(children));

		return {};
	}
}