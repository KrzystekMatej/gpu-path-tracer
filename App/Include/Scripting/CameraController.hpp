#pragma once
#include "ECS/Scene.hpp"
#include "App/Time.hpp"
#include "ECS/Resolvers/Resolver.hpp"

namespace App::Scripting
{
	struct CameraController
	{
		float speed;
		float sensitivity;
	};

	class CameraControllerResolver : public Core::ECS::SceneNodeResolver
	{
	public:
		virtual std::expected<void, Core::Utils::Error> Resolve(
			const Core::ECS::SceneNodeContext& context,
			entt::registry& registry,
			Core::Assets::Manager& assetManager) const override;
	};

	void AwakeCameraController(Core::ECS::Scene& scene);
	void UpdateCameraController(Core::ECS::Scene& registry, const Core::App::Time& time);
}