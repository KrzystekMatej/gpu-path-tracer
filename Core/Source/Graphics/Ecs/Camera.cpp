#include <Core/Graphics/Ecs/Camera.hpp>
#include <Core/Ecs/Transform.hpp>
#include <Core/Utils/Yaml.hpp>

namespace Core::Graphics::Ecs
{
	std::expected<void, Utils::Error> CameraBuilder::Build(
		const Core::Ecs::BuildContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		if (registry.view<Camera>().size() > 0)
			return std::unexpected(Utils::Error("Scene cannot have more than one Camera component"));

		CORE_TRY(fovYDegrees, Utils::Yaml::GetFloat(context.node, "fov-y"));
		CORE_TRY(nearPlane, Utils::Yaml::GetFloat(context.node, "near-plane"));
		CORE_TRY(farPlane, Utils::Yaml::GetFloat(context.node, "far-plane"));

		if (fovYDegrees <= 0.0f || fovYDegrees >= 180.0f)
			return std::unexpected(Utils::Error("Camera 'fov-y' must be in range (0, 180) degrees"));

		if (nearPlane <= 0.0f)
			return std::unexpected(Utils::Error("Camera 'near-plane' must be greater than 0"));

		if (farPlane <= nearPlane)
			return std::unexpected(Utils::Error("Camera 'far-plane' must be greater than 'near-plane'"));

		registry.emplace<Camera>(
			context.entity,
			glm::radians(fovYDegrees),
			nearPlane,
			farPlane);

		return {};
	}
}