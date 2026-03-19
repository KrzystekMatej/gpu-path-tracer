#pragma once
#include "External/Glm.hpp"
#include "ECS/Resolvers/Resolver.hpp"
#include "ECS/Context.hpp"

namespace App::Scripting
{
	struct CameraController
	{
		float speed;
		float sensitivity;

		float yaw = 0.0f;
		float pitch = 0.0f;

		CameraController(float speed, float sensitivity)
			: speed(speed), sensitivity(sensitivity) {}
	};

	class CameraControllerResolver : public Core::ECS::SceneNodeResolver
	{
	public:
		virtual std::expected<void, Core::Utils::Error> Resolve(
			const Core::ECS::SceneNodeContext& context,
			entt::registry& registry,
			Core::Assets::Manager& assetManager) const override;
	};

	void AwakeCameraController(const Core::ECS::Context& context);
	void UpdateCameraController(const Core::ECS::Context& context);
}