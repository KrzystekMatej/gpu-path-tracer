#include <Core/Ecs/Transform.hpp>
#include <Core/Ecs/Hierarchy.hpp>
#include <Core/Utils/Yaml.hpp>
#include <stack>

namespace Core::Ecs
{
	std::expected<void, Utils::Error> TransformBuilder::Build(
		const BuildContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		auto translationResult = Utils::Yaml::GetVec3(context.node, "translation");
		if (!translationResult)
			return std::unexpected(std::move(translationResult).error());

		glm::vec3 translation = translationResult.value();

		auto scaleResult = Utils::Yaml::GetVec3(context.node, "scale");
		if (!scaleResult)
			return std::unexpected(std::move(scaleResult).error());

		glm::vec3 scale = scaleResult.value();

		const bool hasRotation = static_cast<bool>(context.node["rotation"]);
		const bool hasTarget = static_cast<bool>(context.node["target"]);

		if (hasRotation && hasTarget)
			return std::unexpected(Utils::Error("Transform cannot specify both 'rotation' and 'view-at'"));

		glm::quat rotation = glm::identity<glm::quat>();

		if (hasRotation)
		{
			auto eulerResult = Utils::Yaml::GetVec3(context.node, "rotation");
			if (!eulerResult)
				return std::unexpected(eulerResult.error());

			rotation = glm::normalize(glm::quat(glm::radians(eulerResult.value())));
		}
		else if (hasTarget)
		{
			auto targetResult = Utils::Yaml::GetVec3(context.node, "target");
			if (!targetResult)
				return std::unexpected(targetResult.error());

			glm::vec3 forward = targetResult.value() - translation;
			if (glm::length(forward) <= 1e-6f)
				return std::unexpected(Utils::Error("Transform 'target' must differ from 'position'"));

			forward = glm::normalize(forward);

			glm::vec3 up = Utils::Math::CoordinateSystem::Up;
			if (std::abs(glm::dot(forward, up)) > 0.999f)
				up = glm::vec3(0.0f, 0.0f, 1.0f);

			glm::vec3 right = glm::normalize(glm::cross(forward, up));
			up = glm::normalize(glm::cross(right, forward));

			rotation = glm::normalize(glm::quat_cast(glm::mat3(right, up, -forward)));
		}

		registry.emplace<Transform>(context.entity, translation, rotation, scale);
		registry.emplace<WorldTransform>(context.entity);
		return {};
	}

	void PropagateTransform(entt::registry& registry, entt::entity entity, const glm::mat4& parentTransform)
	{
		auto [local, world] = registry.try_get<Transform, WorldTransform>(entity);

		glm::mat4 worldMatrix = parentTransform;

		if (local && world) 
		{
			worldMatrix = parentTransform * local->GetMatrix();
			world->matrix = worldMatrix;
		}

		if (auto children = registry.try_get<Children>(entity))
		{
			for (entt::entity child : children->Get())
			{
				PropagateTransform(registry, child, worldMatrix);
			}
		}
	}

	void UpdateWorldTransforms(Scene& scene)
	{
		auto& registry = scene.GetRegistry();
		registry.view<Transform>(entt::exclude<Parent>)
			.each([&registry](entt::entity entity, const Transform&)
		{
			PropagateTransform(registry, entity, glm::mat4(1.0f));
		});
	}
}