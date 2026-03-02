#pragma once
#include <filesystem>
#include <expected>
#include "Error/Error.hpp"
#include "IO/Image.hpp"


namespace Core::IO
{
	std::expected<Image, Error> LoadImage(const std::filesystem::path& path, Graphics::ColorSpace colorSpace);

	std::expected<ImageMipChain, Error> LoadImageMipChainFromFiles(
		const std::vector<std::filesystem::path>& mipPaths, 
		Graphics::ColorSpace colorSpace);

	std::expected<ImageMipChain, Error> LoadImageMipChainFromFolder(
		const std::filesystem::path& folderPath, 
		Graphics::ColorSpace colorSpace);

	std::expected<Cubemap, Error> LoadCubemapFromFiles(
		const std::array<std::filesystem::path, 6>& facePaths, 
		Graphics::ColorSpace colorSpace);

	std::expected<Cubemap, Error> LoadCubemapFromFolder(
		const std::filesystem::path& folderPath, 
		Graphics::ColorSpace colorSpace, 
		const std::string& prefix);

	std::expected<CubemapMipChain, Error> LoadCubemapMipChainFromFolder(const std::filesystem::path& folderPath, Graphics::ColorSpace colorSpace);
}