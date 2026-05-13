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
		CORE_TRY(translation, Utils::Yaml::GetVec3(context.node, "translation"));
		CORE_TRY(scale, Utils::Yaml::GetVec3(context.node, "scale"));

		const bool hasQuaternion = static_cast<bool>(context.node["quaternion"]);
		const bool hasEulerDegrees = static_cast<bool>(context.node["euler-degrees"]);
		const bool hasEulerRadians = static_cast<bool>(context.node["euler-radians"]);
		const bool hasTarget = static_cast<bool>(context.node["target"]);

		if (hasQuaternion + (hasEulerDegrees || hasEulerRadians) + hasTarget > 1)
			return std::unexpected(Utils::Error("Transform can only have one of 'quaternion', 'euler', or 'target' specified"));

		glm::quat rotation = glm::identity<glm::quat>();

		if (hasQuaternion)
		{
			CORE_TRY(quaternionWxyz, Utils::Yaml::GetVec4(context.node, "quaternion"));
			rotation = glm::normalize(glm::quat(quaternionWxyz.x, quaternionWxyz.y, quaternionWxyz.z, quaternionWxyz.w));
		}
		else if (hasEulerDegrees)
		{
			CORE_TRY(euler, Utils::Yaml::GetVec3(context.node, "euler-degrees"));
			rotation = glm::normalize(glm::quat(glm::radians(euler)));
		}
		else if (hasEulerRadians)
		{
			CORE_TRY(euler, Utils::Yaml::GetVec3(context.node, "euler-radians"));
			rotation = glm::normalize(glm::quat(euler));
		}
		else if (hasTarget)
		{
			CORE_TRY(target, Utils::Yaml::GetVec3(context.node, "target"));
			
			glm::vec3 forward = target - translation;
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