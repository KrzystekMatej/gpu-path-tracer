#pragma once
#include <array>
#include <vector>
#include <Core/Graphics/Common/Formats.hpp>

namespace Core::Import
{
	struct Image
	{
		uint32_t width;
		uint32_t height;
		Graphics::Common::PixelFormat format;
		std::vector<uint8_t> data;
	};

	struct ImageMip
	{
		uint32_t width;
		uint32_t height;
		std::vector<uint8_t> data;
	};

	struct ImageMipChain
	{
		Graphics::Common::PixelFormat format;
		std::vector<ImageMip> mipMaps;
	};


	struct Cubemap
	{
		uint32_t size;
		Graphics::Common::PixelFormat format;
		std::array<std::vector<uint8_t>, 6> faces;
	};

	struct CubemapMip
	{
		uint32_t size;
		std::array<std::vector<uint8_t>, 6> faces;
	};

	struct CubemapMipChain
	{
		Graphics::Common::PixelFormat format;
		std::vector<CubemapMip> mipMaps;
	};
}