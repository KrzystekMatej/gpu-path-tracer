#include <Core/Assets/Key.hpp>
#include <Core/Utils/Hash.hpp>

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

        void HashAppend(Utils::Hasher& hasher, const SubkeyNone&) {}

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
}
