#pragma once
#include <cstdint>
#include <string_view>
#include <variant>
#include <span>
#include "Graphics/Formats.hpp"
#include "Utils/Guid.hpp"

namespace Core::Assets
{
    enum class AssetType
    {
        Texture,
        Mesh,
        Model,
        Shader,
        Material,
        EnvironmentMap,
    };

    struct SubkeyNone {};
    struct SubkeyIndex { uint32_t value; };
    struct SubkeyName  { std::string_view name; };

    using Subkey = std::variant<SubkeyNone, SubkeyIndex, SubkeyName>;

    struct SourcePath
    {
        std::string_view path;
    };

    struct SourcePixel
    {
		Graphics::PixelFormat format;
        std::span<const uint8_t> data;
    };

    using Source = std::variant<SourcePath, SourcePixel>;

    struct Key
    {
        Source source;
        Subkey subkey;
        AssetType type;
    };

    Utils::Guid MakeAssetId(const Key& key);
}
