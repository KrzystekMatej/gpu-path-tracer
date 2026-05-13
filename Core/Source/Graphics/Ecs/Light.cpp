#include <Core/Graphics/Ecs/Light.hpp>
#include <Core/Utils/Yaml.hpp>

namespace Core::Graphics::Ecs
{
	std::expected<Light, Utils::Error> LightBuilder::Extract(YAML::Node node)
	{
		auto colorResult = Utils::Yaml::GetVec3(node, "color");
		if (!colorResult)
			return std::unexpected(Utils::Error(std::move(colorResult).error()));
		auto intensityResult = Utils::Yaml::GetFloat(node, "intensity");
		if (!intensityResult)
			return std::unexpected(Utils::Error(std::move(intensityResult).error()));
		return Light{ colorResult.value(), intensityResult.value() };
	}

	std::expected<void, Utils::Error> LightBuilder::Build(
		const Core::Ecs::BuildContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		auto light = Extract(context.node);
		if (!light)
			return std::unexpected(Utils::Error(std::move(light).error()));
		registry.emplace<Light>(context.entity, light.value());
		return {};
	}
}