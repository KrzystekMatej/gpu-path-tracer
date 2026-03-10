#include "Scripting/CameraController.hpp"
#include "Utils/Yaml.hpp"

namespace App::Scripting
{
	std::expected<void, Core::Utils::Error> CameraControllerResolver::Resolve(
		const Core::ECS::SceneNodeContext& context,
		entt::registry& registry,
		Core::Assets::Manager& assetManager) const
	{
		auto speedResult = Core::Utils::Yaml::GetFloat(context.node, "speed");
		if (!speedResult)
			return std::unexpected(std::move(speedResult).error());

		auto sensitivityResult = Core::Utils::Yaml::GetFloat(context.node, "sensitivity");
		if (!sensitivityResult)
			return std::unexpected(std::move(sensitivityResult).error());

		const float speed = speedResult.value();
		const float sensitivity = sensitivityResult.value();

		if (speed < 0.0f)
			return std::unexpected(Core::Utils::Error("Camera 'speed' must be at least 0"));

		if (sensitivity < 0.0f)
			return std::unexpected(Core::Utils::Error("Camera 'sensitivity' must be at least 0"));

		registry.emplace<CameraController>(context.entity, speed, sensitivity);
		return {};
	}


	void AwakeCameraController(const Core::ECS::Context& context)
	{

	}

	void UpdateCameraController(const Core::ECS::Context& context)
	{

	}
}