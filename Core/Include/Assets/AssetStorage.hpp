#pragma once
#include <unordered_map>
#include <variant>
#include <string_view>
#include <span>
#include <concepts>
#include <expected>
#include <memory>
#include "Error/Error.hpp"

namespace Core::Assets
{
	enum class AssetType
	{
		Texture,
		Mesh,
		Model,
		Shader,
		Material,
		Scene,
	};

	struct Asset {
		virtual ~Asset() = default;
		virtual AssetType GetType() const = 0;
	};

	struct SubkeyNone {};
	struct SubkeyIndex { uint32_t value; };
	struct SubkeyName  { std::string_view name; };

	using AssetSubkey = std::variant<
		SubkeyNone,
		SubkeyIndex,
		SubkeyName
	>;

	struct SourcePath {
		std::string_view path;
	};

	struct SourcePixel {
		uint32_t externalFormat;
		uint32_t pixelType;
		uint32_t internalFormat;
		std::span<const std::byte> data;
	};

	using AssetSource = std::variant<
		SourcePath,
		SourcePixel
	>;

	struct AssetKey
	{
		AssetSource source;
		AssetSubkey subkey;
		AssetType type;
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
		explicit constexpr AssetHandle(uint64_t id) : m_Id(id) {}
		friend class AssetStorage;

		uint64_t m_Id;
	};

	class AssetStorage
	{
	public:
		AssetStorage(const AssetStorage&) = delete;
		AssetStorage& operator=(const AssetStorage&) = delete;
		AssetStorage(AssetStorage&&) noexcept = default;
		AssetStorage& operator=(AssetStorage&&) noexcept = default;
			
		template<AssetDerived T>
		bool Exists(const AssetHandle<T>& handle) const
		{
			return m_Storage.find(handle.m_Id) != m_Storage.end();
		}

		template<AssetDerived T>
		std::expected<const T&, Error> Get(const AssetHandle<T>& handle) const
		{
			auto it = m_Storage.find(handle.m_Id);
			if (it == m_Storage.end())
			{
				return std::unexpected("Asset not found");
			}
			return static_cast<const T&>(it->second);
		}

		template<AssetDerived T>
		std::expected<const T&, Error> GetByKey(const AssetKey& key) const
		{
			uint64_t id = MakeAssetId(key);
			return Get<T>(AssetHandle<T>(id));
		}

		template<AssetDerived T>
		std::expected<AssetHandle<T>, Error> Add(const AssetKey& key, T&& asset)
		{
			uint64_t id = MakeAssetId(key);

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
		uint64_t MakeAssetId(const AssetKey& key) const;
		std::unordered_map<uint64_t, std::unique_ptr<Asset>> m_Storage;
	};
}
