#pragma once
#include <vector>

#include <Core/Assets/Asset.hpp>
#include <Core/Assets/Handle.hpp>
#include <Core/Graphics/Common/Material.hpp>
#include <Core/Graphics/Cpu/Resources/Mesh.hpp>
#include <Core/Graphics/Cpu/Resources/Texture.hpp>
#include <Core/Graphics/Cpu/Resources/EnvironmentMap.hpp>
#include <Core/Graphics/Gl/Resources/EnvironmentMap.hpp>
#include <Core/Graphics/Gl/Resources/Mesh.hpp>
#include <Core/Graphics/Gl/Resources/Texture.hpp>
#include <Core/Graphics/Cuda/Resources/Texture.hpp>
#include <Core/Graphics/Cuda/Resources/EnvironmentMap.hpp>
#include <Core/Graphics/Gl/ShaderProgram.hpp>

namespace Core::Assets
{
    struct Texture : AssetTyped<Texture, AssetType::Texture>
    {
        Texture() = default;
        Texture(Graphics::Cpu::Resources::Texture cpu, Graphics::Gl::Resources::Texture gl, Graphics::Cuda::Resources::Texture cuda)
            : cpu(std::move(cpu)), gl(std::move(gl)), cuda(std::move(cuda)) {}

        Graphics::Cpu::Resources::Texture cpu;
        Graphics::Gl::Resources::Texture gl;
        Graphics::Cuda::Resources::Texture cuda;
    };

    struct Mesh : AssetTyped<Mesh, AssetType::Mesh>
    {
        Mesh() = default;
        Mesh(Graphics::Cpu::Resources::Mesh cpu, Graphics::Gl::Resources::Mesh gl)
            : cpu(std::move(cpu)), gl(std::move(gl)) {}

        Graphics::Cpu::Resources::Mesh cpu;
        Graphics::Gl::Resources::Mesh gl;
    };

    struct EnvironmentMap : AssetTyped<EnvironmentMap, AssetType::EnvironmentMap>
    {
        EnvironmentMap() = default;
        EnvironmentMap(Graphics::Cpu::Resources::EnvironmentMap cpu, Graphics::Gl::Resources::EnvironmentMap gl, Graphics::Cuda::Resources::EnvironmentMap cuda)
            : cpu(std::move(cpu)), gl(std::move(gl)), cuda(std::move(cuda)) {}

        Graphics::Cpu::Resources::EnvironmentMap cpu;
        Graphics::Gl::Resources::EnvironmentMap gl;
        Graphics::Cuda::Resources::EnvironmentMap cuda;
    };

    struct Material : AssetTyped<Material, AssetType::Material>
    {
        Material() = default;
        Material(
            Graphics::Common::SurfaceModel surface,
            Handle<Texture> albedo,
            Handle<Texture> roughness,
            Handle<Texture> metallic,
            Handle<Texture> ao,
            Handle<Texture> normal)
            : surface(surface),
              albedo(albedo),
              roughness(roughness),
              metallic(metallic),
              ao(ao),
              normal(normal) {}

        Graphics::Common::SurfaceModel surface;
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

    struct Model : AssetTyped<Model, AssetType::Model>
    {
        std::vector<ModelPart> parts;
    };

    struct ShaderProgram : AssetTyped<ShaderProgram, AssetType::Shader>
    {
        ShaderProgram() = default;
        ShaderProgram(Graphics::Gl::ShaderProgram program)
            : program(std::move(program)) {}

        Graphics::Gl::ShaderProgram program;
	};
}
