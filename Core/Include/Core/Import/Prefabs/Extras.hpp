#pragma once
#include <string>
#include <optional>
#include <string_view>
#include <array>
#include <Core/Graphics/Common/Material.hpp>

namespace Core::Import
{
    const std::string SurfaceModelKey = "surface";
    const std::string FlipTexturesKey = "flip_textures";

    constexpr std::array<std::pair<std::string_view, Graphics::SurfaceModel>, 9> SurfaceModels = {
        std::pair{ "unlit", Graphics::SurfaceModel::Unlit },
        std::pair{ "normal", Graphics::SurfaceModel::Normal },
        std::pair{ "diffuse", Graphics::SurfaceModel::Diffuse },
        std::pair{ "lambert", Graphics::SurfaceModel::Diffuse },
        std::pair{ "mirror", Graphics::SurfaceModel::Mirror },
        std::pair{ "phong", Graphics::SurfaceModel::Phong },
        std::pair{ "microfacet", Graphics::SurfaceModel::Microfacet },
        std::pair{ "ggx", Graphics::SurfaceModel::Microfacet },
        std::pair{ "emissive", Graphics::SurfaceModel::Emissive }
    };
    
    inline std::optional<Graphics::SurfaceModel> ParseSurfaceModel(std::string_view surfaceValue)
    {
        for (const std::pair<std::string_view, Graphics::SurfaceModel>& surfaceModel : SurfaceModels)
        {
            if (surfaceValue == surfaceModel.first)
                return surfaceModel.second;
        }

        return std::nullopt;
    }  
}