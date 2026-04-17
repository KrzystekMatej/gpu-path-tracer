#include <Core/Ecs/Systems/Render.hpp>
#include <Core/Ecs/Components/Graphics.hpp>
#include <Core/Ecs/Components/Camera.hpp>
#include <Core/Ecs/Components/Transform.hpp>
#include <Core/Graphics/Gl/Material.hpp>

namespace Core::Ecs::Systems
{
	using namespace Components;

	void RenderScene(const Graphics::Gl::Renderer& renderer, const Scene& scene, const Assets::Storage& storage, float aspectRatio)
	{
		const entt::registry& registry = scene.GetRegistry();
		const entt::entity cameraEntity = scene.GetActiveCamera();
		auto [camera, cameraTransform] = registry.get<Camera, WorldTransform>(cameraEntity);

		glm::mat4 projection = camera.GetProjectionMatrix(aspectRatio);
		glm::mat4 cameraModel = cameraTransform.GetMatrix();
		glm::mat4 view = glm::inverse(cameraModel);

		std::vector<Graphics::Common::Light> lights;

		registry.view<WorldTransform, Light>().each([&](const WorldTransform& transform, const Light& light)
		{
			lights.push_back(
			{
				.position = glm::vec3(transform.GetMatrix()[3]),
				.color = light.color,
				.intensity = light.intensity
			});
		});


		const Graphics::Gl::Resources::EnvironmentMap* environmentMap = nullptr;
		registry.view<Background>().each([&](const Background& background)
		{
			environmentMap = &storage.Get(background.handle).value().get().gl;
		});

		registry.view<WorldTransform, Mesh, Material>().each([&](const WorldTransform& transform, const Mesh& mesh, const Material& material)
		{
			glm::mat4 model = transform.GetMatrix();

			const Graphics::Gl::Resources::Mesh& glMesh = storage.Get(mesh.handle).value().get().gl;
			const Assets::Material& materialAsset = storage.Get(material.handle).value().get();
			const Graphics::Gl::Material glMaterial = 
			{
				.localShading = Graphics::Gl::ToLocalShadingUnchecked(materialAsset.surface),
				.albedo = storage.Get(materialAsset.albedo).value().get().gl,
				.roughness = storage.Get(materialAsset.roughness).value().get().gl,
				.metallic = storage.Get(materialAsset.metallic).value().get().gl,
				.ao = storage.Get(materialAsset.ao).value().get().gl,
				.normal = storage.Get(materialAsset.normal).value().get().gl
			};

			Graphics::Gl::DrawContext drawContext(
				storage, 
				model, 
				view, 
				projection, 
				glm::vec3(cameraModel[3]), 
				glMesh, 
				glMaterial, 
				lights,
				environmentMap);

			renderer.DrawMesh(drawContext);
		});

		if (environmentMap)
		{
			 renderer.DrawSkybox(storage, view, projection, environmentMap->GetBackground());
		}
	}
}