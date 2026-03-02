#include <glad/gl.h>
#include "Graphics/Gl/Renderer.hpp"

namespace Core::Graphics::Gl
{
    Renderer::Renderer(const GraphicsContext& context)
		: m_Context(context) {}

    void Renderer::Initialize()
    {
		m_Context.MakeCurrent();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }

    void Renderer::BeginFrame()
    {
		m_Context.MakeCurrent();
    }

    void Renderer::EndFrame()
    {
    }

    void Renderer::Clear(float r, float g, float b, float a)
    {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
}
