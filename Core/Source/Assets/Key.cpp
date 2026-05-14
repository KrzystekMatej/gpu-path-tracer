#include <Core/Assets/Key.hpp>
#include <Core/Utils/Hash.hpp>
#include <string>

namespace Core::Assets
{
    namespace
    {
        void HashAppend(Utils::Hasher& hasher, AssetType type)
        {
            using UT = std::underlying_type_t<AssetType>;
            hasher.Update(static_cast<UT>(type));
        }

        void HashAppend(Utils::Hasher& hasher, const SourcePath& source)
        {
            uint64_t length = source.path.size();
            hasher.Update(length);
            hasher.Update(std::as_bytes(std::span{ source.path }));
        }

        void HashAppend(Utils::Hasher& hasher, const SourcePixel& source)
        {
            hasher.Update(source.format.layout);
            hasher.Update(source.format.componentType);
            hasher.Update(source.format.colorSpace);

            uint64_t length = source.data.size();
            hasher.Update(length);
            hasher.Update(std::as_bytes(source.data));
        }

        void HashAppend(Utils::Hasher&, const SubkeyNone&) {}

        void HashAppend(Utils::Hasher& hasher, const SubkeyIndex& subkey)
        {
            hasher.Update(subkey.value);
        }

        void HashAppend(Utils::Hasher& hasher, const SubkeyName& subkey)
        {
            uint64_t length = subkey.name.size();
            hasher.Update(length);
            hasher.Update(std::as_bytes(std::span{ subkey.name }));
        }

        template<class... Ts>
        void HashAppend(Utils::Hasher& hasher, const std::variant<Ts...>& variant)
        {
            uint8_t index = static_cast<uint8_t>(variant.index());
            hasher.Update(index);
            std::visit([&](const auto& x) { HashAppend(hasher, x); }, variant);
        }
    }

    Utils::Guid MakeAssetId(const Key& key)
    {
        Utils::Hasher hasher;
        HashAppend(hasher, key.type);
        HashAppend(hasher, key.source);
        HashAppend(hasher, key.subkey);
        return hasher.Digest();
    }

    std::string Key::ToString() const
    {
        std::string result;

        std::visit([&](const auto& source) {
            using T = std::decay_t<decltype(source)>;
            if constexpr (std::is_same_v<T, SourcePath>)
                result += "Path: " + std::string(source.path);
            else if constexpr (std::is_same_v<T, SourcePixel>)
                result += "Pixel: " + source.format.ToString() + ", Data size: " + std::to_string(source.data.size());
        }, source);

        result += ", ";

        std::visit([&](const auto& subkey) {
            using T = std::decay_t<decltype(subkey)>;
            if constexpr (std::is_same_v<T, SubkeyNone>)
                result += "Subkey: None";
            else if constexpr (std::is_same_v<T, SubkeyIndex>)
                result += "Subkey: Index " + std::to_string(subkey.value);
            else if constexpr (std::is_same_v<T, SubkeyName>)
                result += "Subkey: Name " + std::string(subkey.name);
        }, subkey);

        result += ", Type: ";

        switch (type)
        {
        case AssetType::Texture:
            result += "Texture";
            break;
        case AssetType::Mesh:
            result += "Mesh";
            break;
        case AssetType::Model:
            result += "Model";
            break;
        case AssetType::Shader:
            result += "Shader";
            break;
        case AssetType::Material:
            result += "Material";
            break;
        case AssetType::EnvironmentMap:
            result += "EnvironmentMap";
            break;
        default:
            result += "Unknown";
            break;
        }

        return result;
    }
}
