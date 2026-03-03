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
	struct Handle
	{
		friend constexpr bool operator==(Handle lhs, Handle rhs)
		{
			return lhs.m_Id == rhs.m_Id;
		}
	private:
		explicit constexpr Handle(AssetId id) : m_Id(id) {}
		friend class Storage;

		AssetId m_Id;
	};

	class Storage
	{
	public:
		Storage() = default;
		Storage(const Storage&) = delete;
		Storage& operator=(const Storage&) = delete;
		Storage(Storage&&) noexcept = default;
		Storage& operator=(Storage&&) noexcept = default;
			
		template<AssetDerived T>
		bool Exists(const Handle<T>& handle) const
		{
			return m_Storage.find(handle.m_Id) != m_Storage.end();
		}

		template<AssetDerived T>
		std::expected<Handle<T>, Utils::Error> GetHandleByKey(const Key& key) const
		{
			AssetId id = MakeAssetId(key);
			if (!m_Storage.contains(id))
				return std::unexpected(Utils::Error("Asset not found"));

			return Handle<T>(id);
		}

		template<AssetDerived T>
		std::expected<const T&, Utils::Error> Get(const Handle<T>& handle) const
		{
			auto it = m_Storage.find(handle.m_Id);
			if (it == m_Storage.end())
				return std::unexpected(Utils::Error("Asset not found"));

			return static_cast<const T&>(*it->second);
		}

		template<AssetDerived T>
		std::expected<const T&, Utils::Error> GetByKey(const Key& key) const
		{
			AssetId id = MakeAssetId(key);
			return Get<T>(Handle<T>(id));
		}

		template<AssetDerived T>
		std::expected<Handle<T>, Utils::Error> Add(const Key& key, T&& asset)
		{
			AssetId id = MakeAssetId(key);

			if (m_Storage.contains(id))
				return std::unexpected(Utils::Error("Asset already exists"));

			m_Storage.emplace(id, std::make_unique<T>(std::forward<T>(asset)));
			return Handle<T>(id);
		}

		template<AssetDerived T>
		bool Remove(const Handle<T>& handle)
		{
			return m_Storage.erase(handle.m_Id) > 0;
		}
	private:
		std::unordered_map<AssetId, std::unique_ptr<Asset>> m_Storage;
	};
}
