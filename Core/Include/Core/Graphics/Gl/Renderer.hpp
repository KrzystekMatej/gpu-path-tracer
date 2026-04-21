#pragma once
#include <Core/Window/GraphicsContext.hpp>
#include <Core/Assets/Manager.hpp>
#include <Core/Graphics/Gl/Resources/Mesh.hpp>
#include <Core/Graphics/Gl/Material.hpp>
#include <Core/Assets/Storage.hpp>
#include <Core/Graphics/Gl/DrawContext.hpp>
#include <Core/Graphics/Gl/RenderSurface.hpp>

namespace Core::Graphics::Gl
{
	class Renderer
    {
    public:
        Renderer(const Window::GraphicsContext& context);
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = default;
		Renderer& operator=(Renderer&&) noexcept = default;

        static std::expected<Renderer, Utils::Error> Create(Assets::Manager& assetManager);

        void InitContext(const Window::GraphicsContext* context);
        void BindSurface(RenderSurface surface);
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const;
		void Clear(float r, float g, float b, float a) const;
		void DrawMesh(const DrawContext& context) const;
		void DrawSkybox(const Assets::Storage& storage, const glm::mat4& view, const glm::mat4& projection, const Texture& skybox) const;
    private:
		Renderer(const Window::GraphicsContext* context,
			Assets::Handle<Assets::ShaderProgram> unlit,
			Assets::Handle<Assets::ShaderProgram> normal,
			Assets::Handle<Assets::ShaderProgram> lambert,
			Assets::Handle<Assets::ShaderProgram> directPbr,
			Assets::Handle<Assets::ShaderProgram> fullPbr,
			Assets::Handle<Assets::ShaderProgram> background,
			Texture brdfMap,
			Mesh skyboxMesh)
		: m_Context(context),
		  m_Unlit(unlit),
		  m_Normal(normal),
		  m_Lambert(lambert),
		  m_DirectPbr(directPbr),
		  m_FullPbr(fullPbr),
		  m_Background(background),
		  m_BrdfMap(std::move(brdfMap)),
	      m_SkyboxMesh(std::move(skyboxMesh)) { }

		const Window::GraphicsContext* m_Context = nullptr;
		Assets::Handle<Assets::ShaderProgram> m_Unlit;
		Assets::Handle<Assets::ShaderProgram> m_Normal;
		Assets::Handle<Assets::ShaderProgram> m_Lambert;
		Assets::Handle<Assets::ShaderProgram> m_DirectPbr;
		Assets::Handle<Assets::ShaderProgram> m_FullPbr;
		Assets::Handle<Assets::ShaderProgram> m_Background;

		Texture m_BrdfMap;
		Mesh m_SkyboxMesh;

		RenderSurface m_RenderSurface;
    };
}