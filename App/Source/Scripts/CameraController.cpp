#include <App/Scripts/CameraController.hpp>
#include <Core/Utils/Yaml.hpp>
#include <Core/ECS/Components/Transform.hpp>

namespace App::Scripts
{
	std::expected<void, Core::Utils::Error> CameraControllerBuilder::Build(
		const Core::ECS::SceneNodes::BuildContext& context,
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
		auto [transform, controller] = context.scene.GetRegistry().get<Core::ECS::Components::Transform, CameraController>(context.scene.GetActiveCamera());
		glm::vec3 forward = transform.GetForward();
		controller.pitch = asinf(forward.y);
		controller.yaw   = atan2f(-forward.x, -forward.z);
	}

	void UpdateCameraController(const Core::ECS::Context& context)
	{
		if (context.input.IsKeyDown(Core::Input::KeyCode::LeftControl))
		{
			if (context.input.WasKeyPressed(Core::Input::KeyCode::LeftControl))
			{
				context.window.SetCursorMode(Core::Window::CursorMode::Disabled);
				context.window.SetRawMouseMotionEnabled(true);
			}
		}
		else
		{
			if (context.input.WasKeyReleased(Core::Input::KeyCode::LeftControl))
			{
				context.window.SetCursorMode(Core::Window::CursorMode::Normal);
				context.window.SetRawMouseMotionEnabled(false);
			}
			else
			{
				return;
			}
		}


		auto [transform, controller] = context.scene.GetRegistry().get<Core::ECS::Components::Transform, CameraController>(context.scene.GetActiveCamera());

		glm::vec3 direction(0.0f);
		if (context.input.IsKeyDown(Core::Input::KeyCode::W))
			direction += transform.GetForward();
		if (context.input.IsKeyDown(Core::Input::KeyCode::S))
			direction -= transform.GetForward();
		if (context.input.IsKeyDown(Core::Input::KeyCode::D))
			direction += transform.GetRight();
		if (context.input.IsKeyDown(Core::Input::KeyCode::A))
			direction -= transform.GetRight();

		if (glm::length(direction) > 0.0f)
			direction = glm::normalize(direction);

		transform.translation += direction * controller.speed * context.time.GetDeltaTime();

		glm::vec2 cursorDelta = context.input.GetCursorDelta() * controller.sensitivity;

		controller.yaw   -= cursorDelta.x;
		controller.pitch -= cursorDelta.y;
		controller.pitch = glm::clamp(controller.pitch, -glm::radians(89.0f), glm::radians(89.0f));

		glm::quat qYaw = glm::angleAxis(controller.yaw, Core::Utils::Math::CoordinateSystem::Up);
		glm::quat qPitch = glm::angleAxis(controller.pitch, Core::Utils::Math::CoordinateSystem::Right);

		transform.rotation = glm::normalize(qYaw * qPitch);
	}
}