#include "ECS/Resolvers/Light.hpp"
#include "Utils/Yaml.hpp"
#include "ECS/Components/Graphics.hpp"

namespace Core::ECS
{
	std::expected<void, Utils::Error> LightResolver::Resolve(
		const SceneNodeContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		auto colorResult = Utils::Yaml::GetVec3(context.node, "color");
		if (!colorResult)
			return std::unexpected(Utils::Error(std::move(colorResult).error()));
		auto intensityResult = Utils::Yaml::GetFloat(context.node, "intensity");
		if (!intensityResult)
			return std::unexpected(Utils::Error(std::move(intensityResult).error()));
		registry.emplace<Components::Light>(context.entity, colorResult.value(), intensityResult.value());
		return {};
	}
}