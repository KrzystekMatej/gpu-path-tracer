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
		Texture() = default;
		Texture(Graphics::Cpu::Texture cpu, Graphics::Gl::Texture gl, Graphics::Cuda::Texture cuda)
			: cpu(std::move(cpu)), gl(std::move(gl)), cuda(std::move(cuda)) {}
		AssetType GetType() const override { return AssetType::Texture; }

		Graphics::Cpu::Texture cpu;
		Graphics::Gl::Texture gl;
		Graphics::Cuda::Texture cuda;
	};

	struct Mesh : public Asset
	{
		Mesh() = default;
		Mesh(Graphics::Cpu::Mesh cpu, Graphics::Gl::Mesh gl)
			: cpu(std::move(cpu)), gl(std::move(gl)) {}
		AssetType GetType() const override { return AssetType::Mesh; }

		Graphics::Cpu::Mesh cpu;
		Graphics::Gl::Mesh gl;
	};

	struct EnvironmentMap : public Asset
	{
		EnvironmentMap() = default;
		EnvironmentMap(Graphics::Cpu::EnvironmentMap cpu, Graphics::Gl::EnvironmentMap gl, Graphics::Cuda::EnvironmentMap cuda)
			: cpu(std::move(cpu)), gl(std::move(gl)), cuda(std::move(cuda)) {}

		AssetType GetType() const override { return AssetType::EnvironmentMap; }

		Graphics::Cpu::EnvironmentMap cpu;
		Graphics::Gl::EnvironmentMap gl;
		Graphics::Cuda::EnvironmentMap cuda;
	};

	struct Material : public Asset
	{
		Material() = default;
		Material(
			Graphics::ShadingModel shader, 
			Handle<Texture> albedo, 
			Handle<Texture> roughness, 
			Handle<Texture> metallic, 
			Handle<Texture> ao, 
			Handle<Texture> normal)
			: shader(shader), albedo(albedo), roughness(roughness), metallic(metallic), ao(ao), normal(normal) {}
		AssetType GetType() const override { return AssetType::Material; }

		Graphics::ShadingModel shader;
		Handle<Texture> albedo;
		Handle<Texture> roughness;
		Handle<Texture> metallic;
		Handle<Texture> ao;
		Handle<Texture> normal;
	};

	struct ModelPart
	{
		Handle<Mesh> mesh;
		Handle<Material> material;
	};

	struct Model : public Asset
	{
		AssetType GetType() const override { return AssetType::Model; }

		std::vector<ModelPart> parts;
	};
}
