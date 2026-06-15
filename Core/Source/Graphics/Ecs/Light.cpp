#include <Core/Graphics/Ecs/Light.hpp>
#include <Core/Utils/Yaml.hpp>

namespace Core::Graphics::Ecs
{
	std::expected<Light, Utils::Error> LightBuilder::Extract(YAML::Node node)
	{
		CORE_TRY(color, Utils::Yaml::GetValue<glm::vec3>(node, "color"));
		CORE_TRY(intensity, Utils::Yaml::GetValue<float>(node, "intensity"));
		return Light{ color, intensity };
	}

	std::expected<void, Utils::Error> LightBuilder::Build(
		const Core::Ecs::BuildContext& context,
		entt::registry& registry,
		Assets::Manager&) const
	{
		CORE_TRY(light, Extract(context.node));
		registry.emplace<Light>(context.entity, light.color, light.intensity);
		return {};
	}
}