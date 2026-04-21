#pragma once
#include <Core/External/Glm.hpp>
#include <Core/Ecs/Builder.hpp>

namespace App::Scripts
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

	class CameraControllerBuilder : public Core::Ecs::Builder
	{
	public:
		virtual std::expected<void, Core::Utils::Error> Build(
			const Core::Ecs::BuildContext& context,
			entt::registry& registry,
			Core::Assets::Manager& assetManager) const override;
	};

	void AwakeCameraController();
	void UpdateCameraController();
}