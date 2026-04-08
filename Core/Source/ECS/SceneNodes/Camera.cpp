#include <Core/ECS/SceneNodes/Camera.hpp>
#include <Core/ECS/Components/Transform.hpp>
#include <Core/ECS/Components/Camera.hpp>
#include <Core/Utils/Yaml.hpp>

namespace Core::ECS::SceneNodes
{
	std::expected<void, Utils::Error> CameraBuilder::Build(
		const BuildContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		if (registry.view<Components::Camera>().size() > 0)
			return std::unexpected(Utils::Error("Scene cannot have more than one Camera component"));

		auto fovYResult = Utils::Yaml::GetFloat(context.node, "fov-y");
		if (!fovYResult)
			return std::unexpected(std::move(fovYResult).error());

		auto nearResult = Utils::Yaml::GetFloat(context.node, "near-plane");
		if (!nearResult)
			return std::unexpected(std::move(nearResult).error());

		auto farResult = Utils::Yaml::GetFloat(context.node, "far-plane");
		if (!farResult)
			return std::unexpected(std::move(farResult).error());

		const float fovYDegrees = fovYResult.value();
		const float nearPlane = nearResult.value();
		const float farPlane = farResult.value();

		if (fovYDegrees <= 0.0f || fovYDegrees >= 180.0f)
			return std::unexpected(Utils::Error("Camera 'fov-y' must be in range (0, 180) degrees"));

		if (nearPlane <= 0.0f)
			return std::unexpected(Utils::Error("Camera 'near-plane' must be greater than 0"));

		if (farPlane <= nearPlane)
			return std::unexpected(Utils::Error("Camera 'far-plane' must be greater than 'near-plane'"));

		registry.emplace<Components::Camera>(
			context.entity,
			glm::radians(fovYDegrees),
			nearPlane,
			farPlane);

		return {};
	}
}