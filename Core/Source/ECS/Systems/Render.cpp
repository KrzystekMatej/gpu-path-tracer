#include "ECS/Systems/Render.hpp"
#include "ECS/Components/Graphics.hpp"
#include "ECS/Components/Camera.hpp"
#include "ECS/Components/Transform.hpp"

namespace Core::ECS::Systems
{
	using namespace Components;

	void RenderScene(const Graphics::Gl::Renderer& renderer, const Scene& scene)
	{
		/*const entt::registry& registry = scene.GetRegistry();
		const entt::entity cameraEntity = scene.GetActiveCamera();
		if (cameraEntity == entt::null)
			return;
		const Camera& camera = registry.get<Camera>(cameraEntity);
		const Transform& cameraTransform = registry.get<Transform>(cameraEntity);
		registry.view<Transform, Mesh, Material>().each([&](const Components::Transform& transform, const Mesh& mesh, const Material& material)
		{
			
		});*/
	}
}