#pragma once
#include <string>
#include <Core/Graphics/Common/Material.hpp>
#include <optional>
#include <Core/Utils/Path.hpp>

namespace Core::Assets
{
    struct ResolvedMaterial
    {
        std::string name;
        Graphics::SurfaceModel surfaceModel;
        float color[3];
        float specular[3];
        float shininess;
        float roughness;
        float metallic;
        float emission[3];
        float ior;
        float transmission;
        float opacity;

        std::optional<Utils::Path::ResolvedPath> colorTexture;
        std::optional<Utils::Path::ResolvedPath> specularTexture;
        std::optional<Utils::Path::ResolvedPath> shininessTexture;
        std::optional<Utils::Path::ResolvedPath> occlusionTexture;
        std::optional<Utils::Path::ResolvedPath> roughnessTexture;
        std::optional<Utils::Path::ResolvedPath> metallicTexture;
        std::optional<Utils::Path::ResolvedPath> rmTexture;
        std::optional<Utils::Path::ResolvedPath> ormTexture;
        std::optional<Utils::Path::ResolvedPath> rmoTexture;
    };
}