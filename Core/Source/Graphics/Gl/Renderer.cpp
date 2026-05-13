#include <glad/gl.h>
#include <Core/Graphics/Gl/Renderer.hpp>
#include <spdlog/spdlog.h>
#include <Core/Graphics/Gl/Shader.hpp>
#include <Core/Import/ImageLoader.hpp>
#include <Core/Import/ObjLoader.hpp>

namespace Core::Graphics::Gl
{
    namespace
    {
        std::array<std::pair<std::filesystem::path, ShaderType>, 2> unlitPaths = 
        {
            {{"Shaders/Unlit/vertex.glsl", ShaderType::Vertex}, {"Shaders/Unlit/fragment.glsl", ShaderType::Fragment}}
		};

        std::array<std::pair<std::filesystem::path, ShaderType>, 2> normalPaths =
        {
            {{"Shaders/Normal/vertex.glsl", ShaderType::Vertex}, {"Shaders/Normal/fragment.glsl", ShaderType::Fragment}}
        };

        std::array<std::pair<std::filesystem::path, ShaderType>, 2> lambertPaths =
        {
            {{"Shaders/Lambert/vertex.glsl", ShaderType::Vertex}, {"Shaders/Lambert/fragment.glsl", ShaderType::Fragment}}
		};

		std::array<std::pair<std::filesystem::path, ShaderType>, 2> phongPaths =
		{
			{ {"Shaders/Phong/vertex.glsl", ShaderType::Vertex}, {"Shaders/Phong/fragment.glsl", ShaderType::Fragment} }
		};

        std::array<std::pair<std::filesystem::path, ShaderType>, 2> directPbrPaths =
        {
            {{"Shaders/DirectPbr/vertex.glsl", ShaderType::Vertex}, {"Shaders/DirectPbr/fragment.glsl", ShaderType::Fragment}}
		};

        std::array<std::pair<std::filesystem::path, ShaderType>, 2> fullPbrPaths =
        {
            {{"Shaders/FullPbr/vertex.glsl", ShaderType::Vertex}, {"Shaders/FullPbr/fragment.glsl", ShaderType::Fragment}}
        };

		std::array<std::pair<std::filesystem::path, ShaderType>, 2> emissivePaths =
		{
			{ {"Shaders/Emissive/vertex.glsl", ShaderType::Vertex}, {"Shaders/Emissive/fragment.glsl", ShaderType::Fragment} }
		};

        std::array<std::pair<std::filesystem::path, ShaderType>, 2> backgroundPaths =
        {
            {{"Shaders/Background/vertex.glsl", ShaderType::Vertex}, {"Shaders/Background/fragment.glsl", ShaderType::Fragment}}
		};

		constexpr std::string_view brdfMapPath = "Backgrounds/brdf_map.exr";
		constexpr std::string_view skyboxPath = "Models/Skybox/mesh.obj";
    }


	void Renderer::InitContext(const Window::GraphicsContext* context)
    {
		m_Context = context;
		m_Context->MakeCurrent();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }

    std::expected<Renderer, Utils::Error> Renderer::Create(Assets::Manager& assetManager)
    {
		CORE_TRY_CONTEXT(unlit, assetManager.ImportShaderProgram(unlitPaths), "Failed to load unlit shader program");
        CORE_TRY_CONTEXT(normal, assetManager.ImportShaderProgram(normalPaths), "Failed to load normal shader program");
		CORE_TRY_CONTEXT(lambert, assetManager.ImportShaderProgram(lambertPaths), "Failed to load lambert shader program");
		CORE_TRY_CONTEXT(phong, assetManager.ImportShaderProgram(phongPaths), "Failed to load phong shader program");
        CORE_TRY_CONTEXT(directPbr, assetManager.ImportShaderProgram(directPbrPaths), "Failed to load direct PBR shader program");
		CORE_TRY_CONTEXT(fullPbr, assetManager.ImportShaderProgram(fullPbrPaths), "Failed to load full PBR shader program");
		CORE_TRY_CONTEXT(emissive, assetManager.ImportShaderProgram(emissivePaths), "Failed to load emissive shader program");
		CORE_TRY_CONTEXT(background, assetManager.ImportShaderProgram(backgroundPaths), "Failed to load background shader program");

		CORE_TRY_CONTEXT(brdfMap, Import::LoadImage(assetManager.GetRootPath() / brdfMapPath, ColorSpace::Linear), "Failed to load BRDF map image");
		CORE_TRY_CONTEXT(brdfMapTexture, Texture::Create2D(brdfMap), "Failed to create BRDF map texture");

		CORE_TRY_CONTEXT(skyboxObj, Import::LoadObj(assetManager.GetRootPath() / skyboxPath), "Failed to load skybox mesh");
		CORE_TRY_CONTEXT(skyboxMesh, Mesh::Create(skyboxObj.meshes[0]), "Failed to create skybox mesh");

		return Renderer(
			nullptr, 
			unlit,
			normal,
			lambert,
			phong,
			directPbr,
			fullPbr,
			emissive,
			background,
			std::move(brdfMapTexture),
			std::move(skyboxMesh));
    }

    void Renderer::BindSurface(RenderSurface surface)
    {
		m_RenderSurface = surface;
		glBindFramebuffer(GL_FRAMEBUFFER, m_RenderSurface.GetFramebufferId());
		SetViewport(0, 0, m_RenderSurface.GetWidth(), m_RenderSurface.GetHeight());
    }
	
	void Renderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const
	{
		glViewport(x, y, width, height);
	}

    void Renderer::Clear(float r, float g, float b, float a) const
    {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void Renderer::DrawMesh(const DrawContext& context) const
    {
		glDepthFunc(GL_LESS);
		glm::mat4 pvm = context.projection * context.view * context.model;
		glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(context.model)));

        switch (context.material.localShading)
        {
			case LocalShadingModel::Unlit:
			{
				const ShaderProgram& program = context.storage.Get(m_Unlit).value().get().program;
				program.Bind();

				glActiveTexture(GL_TEXTURE0);
				context.material.albedo.Bind();
				program.SetInt32("albedo_texture", 0);

				program.SetMatrix4x4("pvm_matrix", pvm);
				program.SetMatrix4x4("model_matrix", context.model);

				context.mesh.BindVertexArray();
				glDrawElements(GL_TRIANGLES, context.mesh.GetVertexCount(), GL_UNSIGNED_INT, nullptr);
				break;
			}
			case LocalShadingModel::Normal:
			{
				const ShaderProgram& program = context.storage.Get(m_Normal).value().get().program;
				program.Bind();

				glActiveTexture(GL_TEXTURE0);
				context.material.normal.Bind();
				program.SetInt32("normal_texture", 0);

				program.SetMatrix4x4("pvm_matrix", pvm);
				program.SetMatrix4x4("model_matrix", context.model);
				program.SetMatrix3x3("normal_matrix", normal);

				context.mesh.BindVertexArray();
				glDrawElements(GL_TRIANGLES, context.mesh.GetVertexCount(), GL_UNSIGNED_INT, nullptr);
				break;
			}
			case LocalShadingModel::Lambert:
			{
				const ShaderProgram& program = context.storage.Get(m_Lambert).value().get().program;

				program.Bind();
				glActiveTexture(GL_TEXTURE0);
				context.material.albedo.Bind();
				program.SetInt32("albedo_texture", 0);
				glActiveTexture(GL_TEXTURE0 + 1);
				context.material.normal.Bind();
				program.SetInt32("normal_texture", 1);

				program.SetMatrix4x4("pvm_matrix", pvm);
				program.SetMatrix4x4("model_matrix", context.model);
				program.SetMatrix3x3("normal_matrix", normal);

				program.SetUInt32("light_count", static_cast<uint32_t>(context.lights.size()));
				for (size_t i = 0; i < context.lights.size(); i++)
				{
					const Light& light = context.lights[i];
					program.SetVec3("lights[" + std::to_string(i) + "].position", light.position);
					program.SetVec3("lights[" + std::to_string(i) + "].color", light.color);
					program.SetFloat("lights[" + std::to_string(i) + "].intensity", light.intensity);
				}

				context.mesh.BindVertexArray();
				glDrawElements(GL_TRIANGLES, context.mesh.GetVertexCount(), GL_UNSIGNED_INT, nullptr);
				break;
			}
			case LocalShadingModel::Phong:
			{
				const ShaderProgram& program = context.storage.Get(m_Phong).value().get().program;

				program.Bind();
				glActiveTexture(GL_TEXTURE0);
				context.material.albedo.Bind();
				program.SetInt32("albedo_texture", 0);
				glActiveTexture(GL_TEXTURE0 + 1);
				context.material.specular.Bind();
				program.SetInt32("specular_texture", 1);
				glActiveTexture(GL_TEXTURE0 + 2);
				context.material.shininess.Bind();
				program.SetInt32("shininess_texture", 2);
				glActiveTexture(GL_TEXTURE0 + 3);
				context.material.normal.Bind();
				program.SetInt32("normal_texture", 3);

				program.SetMatrix4x4("pvm_matrix", pvm);
				program.SetMatrix4x4("model_matrix", context.model);
				program.SetMatrix3x3("normal_matrix", normal);
				program.SetVec3("camera_position", context.cameraPosition);

				program.SetUInt32("light_count", static_cast<uint32_t>(context.lights.size()));
				for (size_t i = 0; i < context.lights.size(); i++)
				{
					const Light& light = context.lights[i];
					program.SetVec3("lights[" + std::to_string(i) + "].position", light.position);
					program.SetVec3("lights[" + std::to_string(i) + "].color", light.color);
					program.SetFloat("lights[" + std::to_string(i) + "].intensity", light.intensity);
				}

				context.mesh.BindVertexArray();
				glDrawElements(GL_TRIANGLES, context.mesh.GetVertexCount(), GL_UNSIGNED_INT, nullptr);
				break;
			}
			case LocalShadingModel::Pbr:
			{
				const ShaderProgram* program = nullptr;
				if (context.environmentMap)
				{
					program = &context.storage.Get(m_FullPbr).value().get().program;
					program->Bind();
					
					glActiveTexture(GL_TEXTURE0 + 5);
					context.environmentMap->GetIrradianceMap().Bind();
					program->SetInt32("irradiance_map", 5);
					glActiveTexture(GL_TEXTURE0 + 6);
					context.environmentMap->GetPrefilteredMap().Bind();
					program->SetInt32("prefiltered_map", 6);
					glActiveTexture(GL_TEXTURE0 + 7);
					m_BrdfMap.Bind();
					program->SetInt32("brdf_map", 7);
				}
				else
				{
					program = &context.storage.Get(m_DirectPbr).value().get().program;
					program->Bind();
				}

				glActiveTexture(GL_TEXTURE0);
				context.material.albedo.Bind();
				program->SetInt32("albedo_texture", 0);
				glActiveTexture(GL_TEXTURE0 + 1);
				context.material.roughness.Bind();
				program->SetInt32("roughness_texture", 1);
				glActiveTexture(GL_TEXTURE0 + 2);
				context.material.metallic.Bind();
				program->SetInt32("metallic_texture", 2);
				glActiveTexture(GL_TEXTURE0 + 3);
				context.material.ao.Bind();
				program->SetInt32("ao_texture", 3);
				glActiveTexture(GL_TEXTURE0 + 4);
				context.material.normal.Bind();
				program->SetInt32("normal_texture", 4);

				program->SetMatrix4x4("pvm_matrix", pvm);
				program->SetMatrix4x4("model_matrix", context.model);
				program->SetMatrix3x3("normal_matrix", normal);
				program->SetVec3("camera_position", context.cameraPosition);

				program->SetUInt32("light_count", static_cast<uint32_t>(context.lights.size()));
				for (size_t i = 0; i < context.lights.size(); i++)
				{
					const Light& light = context.lights[i];
					program->SetVec3("lights[" + std::to_string(i) + "].position", light.position);
					program->SetVec3("lights[" + std::to_string(i) + "].color", light.color);
					program->SetFloat("lights[" + std::to_string(i) + "].intensity", light.intensity);
				}
				context.mesh.BindVertexArray();
				glDrawElements(GL_TRIANGLES, context.mesh.GetVertexCount(), GL_UNSIGNED_INT, nullptr);
				break;
			}
			case LocalShadingModel::Emissive:
			{
				const ShaderProgram& program = context.storage.Get(m_Emissive).value().get().program;
				program.Bind();

				glActiveTexture(GL_TEXTURE0);
				context.material.emission.Bind();
				program.SetInt32("emission_texture", 0);

				program.SetMatrix4x4("pvm_matrix", pvm);

				context.mesh.BindVertexArray();
				glDrawElements(GL_TRIANGLES, context.mesh.GetVertexCount(), GL_UNSIGNED_INT, nullptr);
				break;
			}
            default:
                break;
        }
    }

	void Renderer::DrawSkybox(const Assets::Storage& storage, const glm::mat4& view, const glm::mat4& projection, const Texture& skybox) const
	{
		glDepthFunc(GL_LEQUAL);
		const ShaderProgram& program = storage.Get(m_Background).value().get().program;
		program.Bind();

		glActiveTexture(GL_TEXTURE0);
		skybox.Bind();
		program.SetInt32("environment_map", 0);

		program.SetMatrix4x4("view", view);
		program.SetMatrix4x4("projection", projection);
		m_SkyboxMesh.BindVertexArray();
		glDrawElements(GL_TRIANGLES, m_SkyboxMesh.GetVertexCount(), GL_UNSIGNED_INT, nullptr);
	}
}
