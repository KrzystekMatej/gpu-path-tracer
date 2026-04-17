#include <Core/Ecs/SceneNodes/Light.hpp>
#include <Core/Utils/Yaml.hpp>
#include <Core/Ecs/Components/Graphics.hpp>

namespace Core::Ecs::SceneNodes
{
	std::expected<void, Utils::Error> LightBuilder::Build(
		const BuildContext& context,
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