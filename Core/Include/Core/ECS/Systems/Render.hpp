#pragma once
#include <Core/ECS/Scene.hpp>
#include <Core/Graphics/Gl/Renderer.hpp>

namespace Core::ECS::Systems
{
	void RenderScene(const Graphics::Gl::Renderer& renderer, const Scene& scene, const Assets::Storage& storage, float aspectRatio);
}