#include <App/Scripts/CameraController.hpp>
#include <Core/Utils/Yaml.hpp>
#include <Core/Ecs/Transform.hpp>
#include <Core/Runtime/Application.hpp>

namespace App::Scripts
{
	std::expected<void, Core::Utils::Error> CameraControllerBuilder::Build(
		const Core::Ecs::BuildContext& context,
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


	void AwakeCameraController()
	{
		Core::Ecs::Scene& scene = Core::Runtime::Application::Scene();
		auto [transform, controller] = scene.GetRegistry().get<Core::Ecs::Transform, CameraController>(scene.GetActiveCamera());
		glm::vec3 forward = transform.GetForward();
		controller.pitch = asinf(forward.y);
		controller.yaw   = atan2f(-forward.x, -forward.z);
	}

	void UpdateCameraController()
	{
		const auto& input = Core::Runtime::Application::Input();
		auto& window = Core::Runtime::Application::Window();

		if (input.IsKeyDown(Core::Input::KeyCode::LeftControl))
		{
			if (input.WasKeyPressed(Core::Input::KeyCode::LeftControl))
			{
				window.SetCursorMode(Core::Window::CursorMode::Disabled);
				window.SetRawMouseMotionEnabled(true);
			}
		}
		else
		{
			if (input.WasKeyReleased(Core::Input::KeyCode::LeftControl))
			{
				window.SetCursorMode(Core::Window::CursorMode::Normal);
				window.SetRawMouseMotionEnabled(false);
			}
			else
			{
				return;
			}
		}

		auto& scene = Core::Runtime::Application::Scene();

		auto [transform, controller] = scene.GetRegistry().get<Core::Ecs::Transform, CameraController>(scene.GetActiveCamera());

		glm::vec3 direction(0.0f);
		if (input.IsKeyDown(Core::Input::KeyCode::W))
			direction += transform.GetForward();
		if (input.IsKeyDown(Core::Input::KeyCode::S))
			direction -= transform.GetForward();
		if (input.IsKeyDown(Core::Input::KeyCode::D))
			direction += transform.GetRight();
		if (input.IsKeyDown(Core::Input::KeyCode::A))
			direction -= transform.GetRight();

		if (glm::length(direction) > 0.0f)
			direction = glm::normalize(direction);

		const auto& time = Core::Runtime::Application::Time();

		transform.translation += direction * controller.speed * time.GetDeltaTime();

		glm::vec2 cursorDelta = input.GetCursorDelta() * controller.sensitivity;

		controller.yaw   -= cursorDelta.x;
		controller.pitch -= cursorDelta.y;
		controller.pitch = glm::clamp(controller.pitch, -glm::radians(89.0f), glm::radians(89.0f));

		glm::quat qYaw = glm::angleAxis(controller.yaw, Core::Utils::Math::CoordinateSystem::Up);
		glm::quat qPitch = glm::angleAxis(controller.pitch, Core::Utils::Math::CoordinateSystem::Right);

		transform.rotation = glm::normalize(qYaw * qPitch);
	}
}