#pragma once
#include <Core/Graphics/Gl/Renderer.hpp>
#include <Core/Graphics/Gl/RenderTarget.hpp>
#include <Core/Assets/Storage.hpp>
#include <Core/Ecs/Scene.hpp>

namespace Core::Graphics::Services
{
    struct SceneViewDescription
    {
        Gl::RenderTarget& target;
        const Core::Ecs::Scene& scene;
		const Assets::Storage& storage;
        glm::vec4 clearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
    };

    class SceneRenderer
    {
    public:
        SceneRenderer(Gl::Renderer& renderer)
            : m_Renderer(renderer) { }

        void Render(const SceneViewDescription& viewDescription) const;
    private:
        Gl::Renderer& m_Renderer;
    };
}
