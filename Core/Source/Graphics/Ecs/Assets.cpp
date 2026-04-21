#include <Core/Graphics/Ecs/Assets.hpp>
#include <Core/Ecs/Transform.hpp>
#include <Core/Utils/Yaml.hpp>
#include <Core/Ecs/Hierarchy.hpp>

namespace Core::Graphics::Ecs
{
	std::expected<void, Utils::Error> ModelBuilder::Build(
		const Core::Ecs::BuildContext& context,
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
			registry.emplace<Core::Ecs::Parent>(child, context.entity);
			registry.emplace<Core::Ecs::Transform>(
				child,
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::identity<glm::quat>(),
				glm::vec3(1.0f, 1.0f, 1.0f));
			registry.emplace<Core::Ecs::WorldTransform>(child);
			registry.emplace<Mesh>(child, part.mesh);
			registry.emplace<Material>(child, part.material);
		}

		if (!children.empty())
			registry.emplace<Core::Ecs::Children>(context.entity, std::move(children));

		return {};
	}

	std::expected<void, Utils::Error> BackgroundBuilder::Build(
		const Core::Ecs::BuildContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		auto pathResult = Utils::Yaml::GetString(context.node, "path");
		if (!pathResult)
			return std::unexpected(Utils::Error(std::move(pathResult).error()));
		auto envMapHandleResult = assetManager.ImportEnvironmentMap(std::move(pathResult).value(), Graphics::ColorSpace::Linear);
		if (!envMapHandleResult)
			return std::unexpected(Utils::Error(std::move(envMapHandleResult).error()));
		registry.emplace<Background>(context.entity, envMapHandleResult.value());
		return {};
	}
}