#pragma once
#include <entt/entt.hpp>
#include <vector>

namespace Core::Ecs::Components
{
	struct Parent
	{
		Parent() = default;
		Parent(entt::entity entity)
			: entity(entity) {}

		entt::entity Get() const { return entity; }

	private:
		entt::entity entity;
	};

	struct Children
	{
		Children() = default;
		Children(std::vector<entt::entity> entities)
			: entities(std::move(entities)) {}

		const std::vector<entt::entity>& Get() { return entities; }

	private:
		std::vector<entt::entity> entities;
	};
}