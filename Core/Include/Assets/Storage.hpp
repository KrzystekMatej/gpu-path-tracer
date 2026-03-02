#pragma once
#include <unordered_map>
#include <variant>
#include <string_view>
#include <span>
#include <concepts>
#include <expected>
#include <memory>
#include "Assets/Key.hpp"
#include "Utils/Error/Error.hpp"

namespace Core::Assets
{
	struct Asset 
	{
		virtual ~Asset() = default;
		virtual AssetType GetType() const = 0;
	};

	template<typename T>
	concept AssetDerived = std::derived_from<T, Asset>;

	template<AssetDerived T>
	struct AssetHandle
	{
		friend constexpr bool operator==(AssetHandle lhs, AssetHandle rhs)
		{
			return lhs.m_Id == rhs.m_Id;
		}
	private:
		explicit constexpr AssetHandle(AssetId id) : m_Id(id) {}
		friend class Storage;

		AssetId m_Id;
	};

	class Storage
	{
	public:
		Storage(const Storage&) = delete;
		Storage& operator=(const Storage&) = delete;
		Storage(Storage&&) noexcept = default;
		Storage& operator=(Storage&&) noexcept = default;
			
		template<AssetDerived T>
		bool Exists(const AssetHandle<T>& handle) const
		{
			return m_Storage.find(handle.m_Id) != m_Storage.end();
		}

		template<AssetDerived T>
		std::expected<const T&, Utils::Error> Get(const AssetHandle<T>& handle) const
		{
			auto it = m_Storage.find(handle.m_Id);
			if (it == m_Storage.end())
			{
				return std::unexpected("Asset not found");
			}
			return static_cast<const T&>(*it->second);
		}

		template<AssetDerived T>
		std::expected<const T&, Utils::Error> GetByKey(const Key& key) const
		{
			AssetId id = MakeAssetId(key);
			return Get<T>(AssetHandle<T>(id));
		}

		template<AssetDerived T>
		std::expected<AssetHandle<T>, Utils::Error> Add(const Key& key, T&& asset)
		{
			AssetId id = MakeAssetId(key);

			if (m_Storage.contains(id))
			{
				return std::unexpected("Asset already exists");
			}

			m_Storage.emplace(id, std::make_unique<T>(std::forward<T>(asset)));
			return AssetHandle<T>(id);
		}

		template<AssetDerived T>
		bool Remove(const AssetHandle<T>& handle)
		{
			return m_Storage.erase(handle.m_Id) > 0;
		}
	private:
		std::unordered_map<AssetId, std::unique_ptr<Asset>> m_Storage;
	};
}
