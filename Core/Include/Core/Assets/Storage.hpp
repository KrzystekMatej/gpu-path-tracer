#pragma once
#include <expected>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>

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
        std::expected<Handle<T>, Utils::Error> GetHandleByKey(const Key<T>& key) const
        {
            if (!m_Storage.contains(key.GetId()))
                return std::unexpected(Utils::Error("Asset not found"));

            return Handle<T>(key.GetId());
        }

        template<Asset T>
        std::expected<std::reference_wrapper<T>, Utils::Error> Get(Handle<T> handle) const
        {
            auto it = m_Storage.find(handle.GetId());
            if (it == m_Storage.end())
                return std::unexpected(Utils::Error("Asset not found"));

            return std::ref(static_cast<T&>(*it->second.value));
        }

        template<Asset T>
        std::expected<std::reference_wrapper<T>, Utils::Error> GetByKey(const Key<T>& key) const
        {
            return Get<T>(Handle<T>(key.GetId()));
        }

        template<Asset T>
        std::expected<Handle<T>, Utils::Error> Emplace(const Key<T>& key, T&& asset)
        {
            spdlog::trace("Emplacing {} asset with id: {} (type: {})", key.GetDebugName(), key.GetId(), ToString<T::Type>());

            if (m_Storage.contains(key.GetId()))
                return std::unexpected(Utils::Error("Asset already exists"));

			Entry entry{ std::make_unique<T>(std::move(asset)) };

            m_Storage.emplace(key.GetId(), std::move(entry));
            return Handle<T>(key.GetId());
        }

        template<Asset T>
        bool Remove(Handle<T> handle)
        {
            return m_Storage.erase(handle.GetId()) > 0;
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
