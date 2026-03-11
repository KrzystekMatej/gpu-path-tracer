#include "ECS/Systems/Render.hpp"
#include "ECS/Components/Graphics.hpp"
#include "ECS/Components/Camera.hpp"
#include "ECS/Components/Transform.hpp"

namespace Core::ECS::Systems
{
	using namespace Components;

	void RenderScene(const Graphics::Gl::Renderer& renderer, const Scene& scene)
	{
		const entt::registry& registry = scene.GetRegistry();
		const entt::entity cameraEntity = scene.GetActiveCamera();
		auto [camera, cameraTransform] = registry.get<Camera, WorldTransform>(cameraEntity);
		registry.view<WorldTransform, Mesh, Material>().each([&](const Components::WorldTransform& transform, const Mesh& mesh, const Material& material)
		{
			
		});
	}
}