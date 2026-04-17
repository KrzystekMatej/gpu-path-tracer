#include <Core/Ecs/Systems/Transform.hpp>
#include <stack>
#include <Core/Ecs/Components/Transform.hpp>
#include <Core/Ecs/Components/Hierarchy.hpp>

namespace Core::Ecs::Systems
{
	
	void PropagateTransform(entt::registry& registry, entt::entity entity, const glm::mat4& parentTransform)
	{
		auto [local, world] = registry.try_get<Components::Transform, Components::WorldTransform>(entity);

		glm::mat4 worldMatrix = parentTransform;

		if (local && world) 
		{
			worldMatrix = parentTransform * local->GetMatrix();
			world->matrix = worldMatrix;
		}

		if (auto children = registry.try_get<Components::Children>(entity))
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
		registry.view<Components::Transform>(entt::exclude<Components::Parent>)
			.each([&registry](entt::entity entity, const Components::Transform&)
		{
			PropagateTransform(registry, entity, glm::mat4(1.0f));
		});
	}
}