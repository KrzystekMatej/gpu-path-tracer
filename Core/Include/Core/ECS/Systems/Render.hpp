#pragma once
#include <Core/Ecs/Scene.hpp>
#include <Core/Graphics/Gl/Renderer.hpp>

namespace Core::Ecs::Systems
{
	void RenderScene(const Graphics::Gl::Renderer& renderer, const Scene& scene, const Assets::Storage& storage, float aspectRatio);
}