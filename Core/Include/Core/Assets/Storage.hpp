#pragma once
#include <expected>
#include <functional>
#include <memory>
#include <unordered_map>

#include <Core/Utils/Guid.hpp>
#include <Core/Assets/Asset.hpp>
#include <Core/Assets/Handle.hpp>
#include <Core/Assets/Key.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Assets
{
    class Storage
    {
    public:
        Storage() = default;
        Storage(const Storage&) = delete;
        Storage& operator=(const Storage&) = delete;
        Storage(Storage&&) noexcept = default;
        Storage& operator=(Storage&&) noexcept = default;

        template<Asset T>
        bool Exists(Handle<T> handle) const
        {
            return m_Storage.contains(handle.m_Id);
        }

        template<Asset T>
        std::expected<Handle<T>, Utils::Error> GetHandleByKey(const Source& source, const Subkey& subkey) const
        {
            const Utils::Guid id = MakeAssetId(Key{ source, subkey, T::Type });
            if (!m_Storage.contains(id))
                return std::unexpected(Utils::Error("Asset not found"));

            return Handle<T>(id);
        }

        template<Asset T>
        std::expected<std::reference_wrapper<const T>, Utils::Error> Get(Handle<T> handle) const
        {
            auto it = m_Storage.find(handle.m_Id);
            if (it == m_Storage.end())
                return std::unexpected(Utils::Error("Asset not found"));

            return std::cref(static_cast<const T&>(*it->second.value));
        }

        template<Asset T>
        std::expected<std::reference_wrapper<const T>, Utils::Error> GetByKey(const Source& source, const Subkey& subkey) const
        {
            const Utils::Guid id = MakeAssetId(Key{ source, subkey, T::Type });
            return Get<T>(Handle<T>(id));
        }

        template<Asset T>
        std::expected<Handle<T>, Utils::Error> Emplace(const Source& source, const Subkey& subkey, T&& asset)
        {
            const Utils::Guid id = MakeAssetId(Key{ source, subkey, T::Type });

            if (m_Storage.contains(id))
                return std::unexpected(Utils::Error("Asset already exists"));

			Entry entry(std::make_unique<T>(std::forward<T>(asset)));

            m_Storage.emplace(id, std::move(entry));
            return Handle<T>(id);
        }

        template<Asset T>
        bool Remove(Handle<T> handle)
        {
            return m_Storage.erase(handle.m_Id) > 0;
        }

        void Clear()
        {
            m_Storage.clear();
        }

    private:
        struct Entry
        {
            std::unique_ptr<IAsset> value;
        };

        std::unordered_map<Utils::Guid, Entry> m_Storage;
    };
}
