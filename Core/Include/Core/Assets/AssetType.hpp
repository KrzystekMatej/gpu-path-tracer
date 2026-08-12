#pragma once
#include <cassert>
#include <string_view>

namespace Core::Assets
{
    enum class AssetType
    {
        Texture,
        Mesh,
        Shader,
        Material,
        EnvironmentMap,
        ScenePrefab
    };
    
    inline std::string_view ToString(AssetType type)
    {
        switch (type)
        {
        case AssetType::Texture:
            return "Texture";
        case AssetType::Mesh:
            return "Mesh";
        case AssetType::Shader:
            return "Shader";
        case AssetType::Material:
            return "Material";
        case AssetType::EnvironmentMap:
            return "EnvironmentMap";
        case AssetType::ScenePrefab:
            return "ScenePrefab";
        }
        
        assert(false && "Unknown AssetType");
        return "Unknown";
    }
    
    template <AssetType type>
    consteval std::string_view ToString()
    {
        if constexpr (type == AssetType::Texture)
            return "Texture";
        else if constexpr (type == AssetType::Mesh)
            return "Mesh";
        else if constexpr (type == AssetType::Shader)
            return "Shader";
        else if constexpr (type == AssetType::Material)
            return "Material";
        else if constexpr (type == AssetType::EnvironmentMap)
            return "EnvironmentMap";
        else if constexpr (type == AssetType::ScenePrefab)
            return "ScenePrefab";
        else
            static_assert(false, "Unknown AssetType");
    }
}