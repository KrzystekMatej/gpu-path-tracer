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
            Graphics::SurfaceModel surface,
            Handle<Texture> color,
            Handle<Texture> specular,
            Handle<Texture> shininess,
            Handle<Texture> rma,
            Handle<Texture> emission,
            Handle<Texture> normal,
            float ior,
            float transmission,
            float opacity)
            : surface(surface),
              color(color),
              specular(specular),
              shininess(shininess),
              rma(rma),
              emission(emission),
              normal(normal),
              ior(ior),
              transmission(transmission),
              opacity(opacity) {}

        Graphics::SurfaceModel surface;
        Handle<Texture> color;
        Handle<Texture> specular;
        Handle<Texture> shininess;
        Handle<Texture> rma;
        Handle<Texture> emission;
        Handle<Texture> normal;
        float ior;
        float transmission;
        float opacity;
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
