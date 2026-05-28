#include <Core/Graphics/Ecs/Render.hpp>
#include <Core/Graphics/Ecs/Assets.hpp>
#include <Core/Graphics/Ecs/Camera.hpp>
#include <Core/Ecs/Transform.hpp>
#include <Core/Graphics/Gl/Material.hpp>
#include <Core/Graphics/Ecs/Light.hpp>
#include <Core/Graphics/Gl/Light.hpp>

namespace Core::Graphics::Ecs
{
	void RenderScene(const Graphics::Gl::Renderer& renderer, const Core::Ecs::Scene& scene, const Assets::Storage& storage, float aspectRatio)
	{
		const entt::registry& registry = scene.GetRegistry();
		const entt::entity cameraEntity = scene.GetActiveCamera();
		if (cameraEntity == entt::null)
			return;

		auto [camera, cameraTransform] = registry.get<Camera, Core::Ecs::WorldTransform>(cameraEntity);

		glm::mat4 projection = camera.GetProjectionMatrix(aspectRatio);
		glm::mat4 cameraModel = cameraTransform.GetMatrix();
		glm::mat4 view = glm::inverse(cameraModel);

		std::vector<Gl::Light> lights;

		registry.view<Core::Ecs::WorldTransform, Light>().each([&](const Core::Ecs::WorldTransform& transform, const Light& light)
		{
			lights.push_back(
			{
				.position = glm::vec3(transform.GetMatrix()[3]),
				.color = light.color,
				.intensity = light.intensity
			});
		});


		const Graphics::Gl::EnvironmentMap* environmentMap = nullptr;
		registry.view<Background>().each([&](const Background& background)
		{
			environmentMap = &storage.Get(background.handle).value().get().gl;
		});

		registry
			.view<Core::Ecs::WorldTransform, Mesh, Material>()
			.each([&](const Core::Ecs::WorldTransform& transform, const Mesh& mesh, const Material& material)
		{
			glm::mat4 model = transform.GetMatrix();

			const Graphics::Gl::Mesh& glMesh = storage.Get(mesh.handle).value().get().gl;
			const Assets::Material& materialAsset = storage.Get(material.handle).value().get();
			const Graphics::Gl::Material glMaterial = 
			{
				.localShading = Graphics::Gl::ToLocalShadingUnchecked(materialAsset.surface),
				.color = storage.Get(materialAsset.color).value().get().gl,
				.specular = storage.Get(materialAsset.specular).value().get().gl,
				.shininess = storage.Get(materialAsset.shininess).value().get().gl,
				.rma = storage.Get(materialAsset.rma).value().get().gl,
				.normal = storage.Get(materialAsset.normal).value().get().gl,
				.emission = storage.Get(materialAsset.emission).value().get().gl,
				.ior = materialAsset.ior
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