#pragma once
#include <Core/ECS/Scene.hpp>

namespace Core::ECS::Systems
{
	void PropagateTransform(entt::registry& registry, entt::entity entity, const glm::mat4& parentTransform);
	void UpdateWorldTransforms(Scene& scene);
}