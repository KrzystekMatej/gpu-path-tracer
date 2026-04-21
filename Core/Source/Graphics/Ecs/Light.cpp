#include <Core/Graphics/Ecs/Light.hpp>
#include <Core/Utils/Yaml.hpp>

namespace Core::Graphics::Ecs
{
	std::expected<void, Utils::Error> LightBuilder::Build(
		const Core::Ecs::BuildContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		auto colorResult = Utils::Yaml::GetVec3(context.node, "color");
		if (!colorResult)
			return std::unexpected(Utils::Error(std::move(colorResult).error()));
		auto intensityResult = Utils::Yaml::GetFloat(context.node, "intensity");
		if (!intensityResult)
			return std::unexpected(Utils::Error(std::move(intensityResult).error()));
		registry.emplace<Light>(context.entity, colorResult.value(), intensityResult.value());
		return {};
	}
}