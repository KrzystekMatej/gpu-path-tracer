#include "Assets/AssetStorage.hpp"
#include "Utils/Hasher.hpp"

namespace Core::Assets
{
	namespace
	{
		void HashAssetType(Utils::Hasher& hasher, AssetType type)
		{
			using UT = std::underlying_type_t<AssetType>;
			hasher.Update(static_cast<UT>(type));
		}

		void HashAssetSource(Utils::Hasher& hasher, const AssetSource& source)
		{
			uint8_t index = static_cast<uint8_t>(source.index());
			hasher.Update(index);

			std::visit([&](const auto& sourceVariant)
			{
				using T = std::decay_t<decltype(sourceVariant)>;

				if constexpr (std::is_same_v<T, SourcePath>)
				{
					uint64_t length = sourceVariant.path.size();
					hasher.Update(length);
					hasher.Update(std::as_bytes(std::span(sourceVariant.path)));
				}
				else if constexpr (std::is_same_v<T, SourcePixel>)
				{
					hasher.Update(sourceVariant.externalFormat);
					hasher.Update(sourceVariant.pixelType);
					hasher.Update(sourceVariant.internalFormat);

					uint64_t length = sourceVariant.data.size();
					hasher.Update(length);
					hasher.Update(sourceVariant.data);
				}

			}, source);
		}

		void HashAssetSubkey(Utils::Hasher& hasher, const AssetSubkey& subkey)
		{
			uint8_t index = static_cast<uint8_t>(subkey.index());
			hasher.Update(index);

			std::visit([&](const auto& subkeyVariant)
			{
				using T = std::decay_t<decltype(subkeyVariant)>;

				if constexpr (std::is_same_v<T, SubkeyIndex>)
				{
					hasher.Update(subkeyVariant.value);
				}
				else if constexpr (std::is_same_v<T, SubkeyName>)
				{
					uint64_t length = subkeyVariant.name.size();
					hasher.Update(length);
					hasher.Update(std::as_bytes(std::span(subkeyVariant.name)));
				}
			}, subkey);
		}
	}


	uint64_t AssetStorage::MakeAssetId(const AssetKey& key) const
	{
		Utils::Hasher hasher;

		HashAssetType(hasher, key.type);
		HashAssetSource(hasher, key.source);
		HashAssetSubkey(hasher, key.subkey);

		return hasher.Digest();
	}
}