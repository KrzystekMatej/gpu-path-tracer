#include <glad/gl.h>
#include <Core/Graphics/Services/SceneRenderer.hpp>
#include <Core/Graphics/Ecs/Render.hpp>


namespace Core::Graphics::Services
{
    void SceneRenderer::Render(const SceneViewDescription& viewDescription) const
    {
        if (viewDescription.target.GetWidth() == 0 || viewDescription.target.GetHeight() == 0)
            return;

        m_Renderer.BindSurface(viewDescription.target.GetRenderSurface());
        m_Renderer.Clear(viewDescription.clearColor.r, viewDescription.clearColor.g, viewDescription.clearColor.b, viewDescription.clearColor.a);

        float aspect = viewDescription.target.GetWidth() / static_cast<float>(viewDescription.target.GetHeight());
        Ecs::RenderScene(m_Renderer, viewDescription.scene, viewDescription.storage, aspect);
        viewDescription.target.Resolve();
    }
}