#pragma once
#include <array>
#include <vector>
#include "Graphics/Formats.hpp"

namespace Core::IO
{
	struct Image
	{
		uint32_t width;
		uint32_t height;
		Graphics::PixelFormat format;
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
		Graphics::PixelFormat format;
		std::vector<ImageMip> mipMaps;
	};


	struct Cubemap
	{
		uint32_t size;
		Graphics::PixelFormat format;
		std::array<std::vector<uint8_t>, 6> faces;
	};

	struct CubemapMip
	{
		uint32_t size;
		std::array<std::vector<uint8_t>, 6> faces;
	};

	struct CubemapMipChain
	{
		Graphics::PixelFormat format;
		std::vector<CubemapMip> mipMaps;
	};
}