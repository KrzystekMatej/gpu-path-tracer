#include "Assets/Key.hpp"
#include "Utils/Hasher.hpp"

namespace Core::Assets
{
    namespace
    {
        void HashAppend(Utils::Hasher& h, AssetType type)
        {
            using UT = std::underlying_type_t<AssetType>;
            h.Update(static_cast<UT>(type));
        }

        void HashAppend(Utils::Hasher& h, const SourcePath& s)
        {
            uint64_t length = s.path.size();
            h.Update(length);
            h.Update(std::as_bytes(std::span{ s.path }));
        }

        void HashAppend(Utils::Hasher& h, const SourcePixel& s)
        {
            h.Update(s.externalFormat);
            h.Update(s.pixelType);
            h.Update(s.internalFormat);

            uint64_t length = s.data.size();
            h.Update(length);
            h.Update(s.data);
        }

        void HashAppend(Utils::Hasher& h, const SubkeyNone&) {}

        void HashAppend(Utils::Hasher& h, const SubkeyIndex& s)
        {
            h.Update(s.value);
        }

        void HashAppend(Utils::Hasher& h, const SubkeyName& s)
        {
            uint64_t length = s.name.size();
            h.Update(length);
            h.Update(std::as_bytes(std::span{ s.name }));
        }

        template<class... Ts>
        void HashAppend(Utils::Hasher& h, const std::variant<Ts...>& v)
        {
            uint8_t index = static_cast<uint8_t>(v.index());
            h.Update(index);
            std::visit([&](const auto& x) { HashAppend(h, x); }, v);
        }
    }

    AssetId MakeAssetId(const Key& key)
    {
        Utils::Hasher h;
        HashAppend(h, key.type);
        HashAppend(h, key.source);
        HashAppend(h, key.subkey);
        return h.Digest();
    }
}
