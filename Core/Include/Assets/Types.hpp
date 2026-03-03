#pragma once
#include <vector>

#include "Assets/Asset.hpp"
#include "Assets/Handle.hpp"
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
    struct Texture : AssetTyped<Texture, AssetType::Texture>
    {
        Texture() = default;
        Texture(Graphics::Cpu::Texture cpu, Graphics::Gl::Texture gl, Graphics::Cuda::Texture cuda)
            : cpu(std::move(cpu)), gl(std::move(gl)), cuda(std::move(cuda)) {}

        Graphics::Cpu::Texture cpu;
        Graphics::Gl::Texture gl;
        Graphics::Cuda::Texture cuda;
    };

    struct Mesh : AssetTyped<Mesh, AssetType::Mesh>
    {
        Mesh() = default;
        Mesh(Graphics::Cpu::Mesh cpu, Graphics::Gl::Mesh gl)
            : cpu(std::move(cpu)), gl(std::move(gl)) {}

        Graphics::Cpu::Mesh cpu;
        Graphics::Gl::Mesh gl;
    };

    struct EnvironmentMap : AssetTyped<EnvironmentMap, AssetType::EnvironmentMap>
    {
        EnvironmentMap() = default;
        EnvironmentMap(Graphics::Cpu::EnvironmentMap cpu, Graphics::Gl::EnvironmentMap gl, Graphics::Cuda::EnvironmentMap cuda)
            : cpu(std::move(cpu)), gl(std::move(gl)), cuda(std::move(cuda)) {}

        Graphics::Cpu::EnvironmentMap cpu;
        Graphics::Gl::EnvironmentMap gl;
        Graphics::Cuda::EnvironmentMap cuda;
    };

    struct Material : AssetTyped<Material, AssetType::Material>
    {
        Material() = default;
        Material(
            Graphics::ShadingModel shader,
            Handle<Texture> albedo,
            Handle<Texture> roughness,
            Handle<Texture> metallic,
            Handle<Texture> ao,
            Handle<Texture> normal)
            : shader(shader),
              albedo(albedo),
              roughness(roughness),
              metallic(metallic),
              ao(ao),
              normal(normal) {}

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

    struct Model : AssetTyped<Model, AssetType::Model>
    {
        std::vector<ModelPart> parts;
    };
}
