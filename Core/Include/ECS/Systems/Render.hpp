#pragma once
#include "ECS/Scene.hpp"
#include "Graphics/Gl/Renderer.hpp"

namespace Core::ECS::Systems
{
	void RenderScene(float aspectRatio, const Graphics::Gl::Renderer& renderer, const Scene& scene, const Assets::Storage& storage);
}