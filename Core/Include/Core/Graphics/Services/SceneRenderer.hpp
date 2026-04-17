#pragma once
#include <Core/Graphics/Gl/Renderer.hpp>
#include <Core/Graphics/Gl/RenderTarget.hpp>
#include <Core/Assets/Storage.hpp>
#include <Core/Ecs/Scene.hpp>

namespace Core::Graphics::Services
{
    struct SceneViewDesc
    {
        Gl::RenderTarget& target;
        const Ecs::Scene& scene;
        glm::vec4 clearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
    };

    class SceneRenderer
    {
    public:
        SceneRenderer(Gl::Renderer& renderer, const Assets::Storage& storage)
            : m_Renderer(renderer), m_Storage(storage) { }

        void Render(const SceneViewDesc& desc) const;
    private:
        Gl::Renderer& m_Renderer;
		const Assets::Storage& m_Storage;
    };
}
