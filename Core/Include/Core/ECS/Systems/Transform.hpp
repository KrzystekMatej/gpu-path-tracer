#pragma once
#include <Core/Ecs/Scene.hpp>

namespace Core::Ecs::Systems
{
	void PropagateTransform(entt::registry& registry, entt::entity entity, const glm::mat4& parentTransform);
	void UpdateWorldTransforms(Scene& scene);
}