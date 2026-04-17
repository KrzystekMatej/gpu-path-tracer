#include <Core/Graphics/Services/SceneRenderer.hpp>
#include <Core/Ecs/Systems/Render.hpp>


namespace Core::Graphics::Services
{
    void SceneRenderer::Render(const SceneViewDesc& desc) const
    {
        if (desc.target.GetWidth() == 0 || desc.target.GetHeight() == 0)
            return;

        m_Renderer.BindSurface(desc.target.GetRenderSurface());
        m_Renderer.Clear(desc.clearColor.r, desc.clearColor.g, desc.clearColor.b, desc.clearColor.a);

        float aspect = desc.target.GetWidth() / static_cast<float>(desc.target.GetHeight());
        Ecs::Systems::RenderScene(m_Renderer, desc.scene, m_Storage, aspect);
    }
}