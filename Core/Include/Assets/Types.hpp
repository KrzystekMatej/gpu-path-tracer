#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Assets/Storage.hpp"
#include "Graphics/MaterialDefaults.hpp"
#include "Graphics/Cpu/Resources/Mesh.hpp"
#include "Graphics/Cpu/Resources/Texture.hpp"
#include "Graphics/Cpu/Resources/EnvironmentMap.hpp"
#include "Graphics/Gl/Resources/EnvironmentMap.hpp"
#include "Graphics/Gl/Resources/Mesh.hpp"
#include "Graphics/Gl/Resources/Texture.hpp"
#include "Graphics/Cuda/Resources.hpp"

namespace Core::Assets
{
	struct Texture : public Asset
	{
		Graphics::Cpu::Texture cpu;
		Graphics::Gl::Texture gl;
		Graphics::Cuda::Texture cuda;
	};

	struct Mesh : public Asset
	{
		Graphics::Cpu::Mesh cpu;
		Graphics::Gl::Mesh gl;
	};

	struct EnvironmentMap : public Asset
	{
		Graphics::Cpu::EnvironmentMap cpu;
		Graphics::Gl::EnvironmentMap gl;
		Graphics::Cuda::EnvironmentMap cuda;
	};

	struct Material : public Asset
	{
		Graphics::ShadingModel shader;
		AssetHandle<Texture> albedo;
		AssetHandle<Texture> roughness;
		AssetHandle<Texture> metallic;
		AssetHandle<Texture> ao;
		AssetHandle<Texture> normal;
	};

	struct ModelPart
	{
		AssetHandle<Mesh> mesh;
		AssetHandle<Material> material;
	};

	struct Model : public Asset
	{
		std::vector<ModelPart> parts;
	};
}
