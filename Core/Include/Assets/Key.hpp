#pragma once
#include <unordered_map>
#include <variant>
#include <string_view>
#include <span>
#include <concepts>
#include <expected>
#include <memory>

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

	struct SubkeyNone {};
	struct SubkeyIndex { uint32_t value; };
	struct SubkeyName  { std::string_view name; };

	using Subkey = std::variant<SubkeyNone, SubkeyIndex, SubkeyName>;

	struct SourcePath 
	{
		std::string_view path;
	};

	struct SourcePixel 
	{
		uint32_t externalFormat;
		uint32_t pixelType;
		uint32_t internalFormat;
		std::span<const std::byte> data;
	};

	using Source = std::variant<SourcePath, SourcePixel>;

	struct Key
	{
		Source source;
		Subkey subkey;
		AssetType type;
	};

	using AssetId = uint64_t;

	AssetId MakeAssetId(const Key& key);
}
