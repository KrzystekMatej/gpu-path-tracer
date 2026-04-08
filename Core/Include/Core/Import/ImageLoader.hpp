#pragma once
#include <filesystem>
#include <expected>
#include <Core/Utils/Error.hpp>
#include <Core/Import/Image.hpp>


namespace Core::Import
{
	std::expected<Image, Utils::Error> LoadImage(const std::filesystem::path& path, Graphics::Common::ColorSpace colorSpace);

	std::expected<ImageMipChain, Utils::Error> LoadImageMipChainFromFiles(
		const std::vector<std::filesystem::path>& mipPaths, 
		Graphics::Common::ColorSpace colorSpace);
	std::expected<ImageMipChain, Utils::Error> LoadImageMipChainFromFolder(
		const std::filesystem::path& folderPath, 
		Graphics::Common::ColorSpace colorSpace);

	std::expected<Cubemap, Utils::Error> LoadCubemapFromFiles(
		const std::array<std::filesystem::path, 6>& facePaths, 
		Graphics::Common::ColorSpace colorSpace);
	std::expected<Cubemap, Utils::Error> LoadCubemapFromFolder(
		const std::filesystem::path& folderPath, 
		Graphics::Common::ColorSpace colorSpace, 
		const std::string& prefix);

	std::expected<CubemapMipChain, Utils::Error> LoadCubemapMipChainFromFolder(
		const std::filesystem::path& folderPath, 
		Graphics::Common::ColorSpace colorSpace);
}