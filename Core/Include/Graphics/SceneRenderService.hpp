#pragma once
#include "Graphics/Gl/Renderer.hpp"
#include "Graphics/Gl/RenderTarget.hpp"
#include "Assets/Storage.hpp"
#include "ECS/Scene.hpp"

namespace Core::Graphics
{
    struct SceneViewDesc
    {
        Gl::RenderTarget& target;
        const ECS::Scene& scene;
        glm::vec4 clearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
    };

    class SceneRenderService
    {
    public:
        SceneRenderService(Gl::Renderer& renderer, const Assets::Storage& storage)
            : m_Renderer(renderer), m_Storage(storage) { }

        void Render(const SceneViewDesc& desc) const;
    private:
        Gl::Renderer& m_Renderer;
		const Assets::Storage& m_Storage;
    };
}
