#include "Assets/AssetStorage.hpp"
#include "Utils/Hasher.hpp"

namespace Core::Assets
{
	uint64_t AssetStorage::MakeAssetId(const AssetKey& key)
	{
		m_Hasher.Reset();

		HashAssetType(key.type);
		HashAssetSource(key.source);
		HashAssetSubkey(key.subkey);

		return m_Hasher.Digest();
	}

	void AssetStorage::HashAssetType(AssetType type)
	{
		using UT = std::underlying_type_t<AssetType>;
		m_Hasher.Update(static_cast<UT>(type));
	}

	void AssetStorage::HashAssetSource(const AssetSource& source)
	{
		uint8_t index = static_cast<uint8_t>(source.index());
		m_Hasher.Update(index);

		std::visit([&](const auto& sourceVariant)
		{
			using T = std::decay_t<decltype(sourceVariant)>;

			if constexpr (std::is_same_v<T, SourcePath>)
			{
				uint64_t length = sourceVariant.path.size();
				m_Hasher.Update(length);
				m_Hasher.Update(std::as_bytes(std::span(sourceVariant.path)));
			}
			else if constexpr (std::is_same_v<T, SourcePixel>)
			{
				m_Hasher.Update(sourceVariant.externalFormat);
				m_Hasher.Update(sourceVariant.pixelType);
				m_Hasher.Update(sourceVariant.internalFormat);

				uint64_t length = sourceVariant.data.size();
				m_Hasher.Update(length);
				m_Hasher.Update(sourceVariant.data);
			}

		}, source);
	}

	void AssetStorage::HashAssetSubkey(const AssetSubkey& subkey)
	{
		uint8_t index = static_cast<uint8_t>(subkey.index());
		m_Hasher.Update(index);

		std::visit([&](const auto& subkeyVariant)
		{
			using T = std::decay_t<decltype(subkeyVariant)>;

			if constexpr (std::is_same_v<T, SubkeyIndex>)
			{
				m_Hasher.Update(subkeyVariant.value);
			}
			else if constexpr (std::is_same_v<T, SubkeyName>)
			{
				uint64_t length = subkeyVariant.name.size();
				m_Hasher.Update(length);
				m_Hasher.Update(std::as_bytes(std::span(subkeyVariant.name)));
			}
		}, subkey);
	}
}