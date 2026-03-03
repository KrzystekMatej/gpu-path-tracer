#include "IO/ImageLoader.hpp"
#include <string_view>
#include <algorithm>
#include <ranges>
#include <OpenImageIO/imageio.h>
#include "Utils/Text.hpp"

namespace Core::IO
{
	std::expected<Image, Utils::Error> LoadImage(const std::filesystem::path& path, Graphics::ColorSpace colorSpace)
	{
		std::unique_ptr<OIIO::ImageInput> imageInput = OIIO::ImageInput::open(path);

		if (!imageInput)
		{
			return std::unexpected(Utils::Error("{}, file path: {}", OIIO::geterror(), path.string()));
		}

		const OIIO::ImageSpec& spec = imageInput->spec();

		uint32_t width = static_cast<uint32_t>(spec.width);
		uint32_t height = static_cast<uint32_t>(spec.height);
		uint8_t channels = static_cast<uint8_t>(spec.nchannels);
		Graphics::PixelFormat format;
		format.colorSpace = colorSpace;

		switch (channels)
		{
			case 1:
				format.layout = Graphics::ChannelLayout::R;
				break;
			case 3:
				format.layout = Graphics::ChannelLayout::RGB;
				break;
			case 4:
				format.layout = Graphics::ChannelLayout::RGBA;
				break;
			default:
				return std::unexpected(Utils::Error("Unsupported number of channels: {}, file path: {}", spec.nchannels, path.string()));
		}

		switch (spec.format.basetype)
		{
			case OIIO::TypeDesc::UINT8:
				format.componentType = Graphics::ComponentType::UInt8;
				break;
			case OIIO::TypeDesc::HALF:
				format.componentType = Graphics::ComponentType::Float16;
				break;
			case OIIO::TypeDesc::FLOAT:
				format.componentType = Graphics::ComponentType::Float32;
				break;
			default:
				return std::unexpected(Utils::Error("Unsupported pixel type: {}, file path: {}", spec.format.c_str(), path.string()));
		}

		std::vector<uint8_t> pixels(width * height * channels * spec.format.size());

		imageInput->read_image(0, 0, 0, channels, spec.format, pixels.data());


		return Image
		{
			.width = width,
			.height = height,
			.format = format,
			.data = std::move(pixels)
		};
	}

	std::expected<ImageMipChain, Utils::Error> LoadImageMipChainFromFiles(const std::vector<std::filesystem::path>& mipPaths, Graphics::ColorSpace colorSpace)
	{
		uint32_t width = 1;
		uint32_t height = 1;
		ImageMipChain mipChain;

		for (size_t i = 0; i < mipPaths.size(); i++)
		{
			auto& path = mipPaths[i];
			auto imageResult = LoadImage(path, colorSpace);
			if (!imageResult)
			{
				return std::unexpected(Utils::Error(
						"Failed to load mip level {}, error: {}, file path: {}",
						i,
						imageResult.error().Message(),
						path.string()));
			}

			Image image = std::move(imageResult.value());

			if (i == 0)
			{
				width = image.width;
				height = image.height;
				mipChain.format = image.format;
			}
			else
			{
				width = std::max(1u, width / 2);
				height = std::max(1u, height / 2);
				if (image.width != width || image.height != height)
				{
					return std::unexpected(Utils::Error(
							"Invalid mip level {}, expected dimensions {}x{}, but got {}x{}, file path: {}",
							i,
							width,
							height,
							image.width,
							image.height,
							path.string()));
				}

				if (mipChain.format != image.format)
				{
					return std::unexpected(Utils::Error(
						"Invalid mip level {}, pixel format mismatch, file path: {}",
						i,
						path.string()));
				}
			}

			mipChain.mipMaps.push_back(ImageMip
			{
				.width = image.width,
				.height = image.height,
				.data = std::move(image.data)
			});
		}
		return mipChain;
	}


	std::expected<ImageMipChain, Utils::Error> LoadImageMipChainFromFolder(const std::filesystem::path& folderPath, Graphics::ColorSpace colorSpace)
	{
		if (!std::filesystem::exists(folderPath))
			return std::unexpected(Utils::Error("Path {} does not exist", folderPath.string()));

		if (!std::filesystem::is_directory(folderPath))
			return std::unexpected(Utils::Error("Path {} is not a directory", folderPath.string()));

		std::vector<std::pair<uint32_t, std::filesystem::path>> mipPaths;
		std::string extension = "";

		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(folderPath)) {
			if (!entry.is_regular_file())
				continue;

			const std::string filename = entry.path().filename().string();
			if (filename[0] != 'm')
				continue;

			std::filesystem::path fileExtension = entry.path().extension();

			if (fileExtension.empty())
			{
				return std::unexpected(Utils::Error(
						"File {} in mip chain folder does not have an extension, "
						"expected all files to have the same extension {}, folder path: {}",
						entry.path().string(),
						extension,
						folderPath.string()));
			}

			if (extension.empty())
			{
				extension = entry.path().extension().string();
			}
			else if (entry.path().extension() != extension)
			{
				return std::unexpected(Utils::Error(
						"Inconsistent file extensions in mip chain folder, "
						"expected all files to have the same extension {}, "
						"but found file with extension {}, folder path: {}",
						extension,
						entry.path().extension().string(),
						folderPath.string()));
			}

			std::string_view index = std::string_view(filename).substr(1, filename.size() - 1);

			if (!Utils::Text::IsNumeric(index)) continue;
			mipPaths.emplace_back(std::stoul(std::string(index)), entry.path());
		}


		std::sort(mipPaths.begin(), mipPaths.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
		bool ok = !mipPaths.empty() && std::ranges::equal(
			mipPaths | std::views::transform([](const auto& p) { return p.first; }),
			std::views::iota(uint32_t{0}, static_cast<uint32_t>(mipPaths.size())));

		if (!ok)
		{
			return std::unexpected(Utils::Error(
					"Invalid mip chain folder structure, expected files named m0, m1...mN with no gaps, folder path: {}",
					folderPath.string()));
		}

		std::vector<std::filesystem::path> sortedMipPaths(mipPaths.size());
		for (size_t i = 0; i < mipPaths.size(); i++)
		{
			sortedMipPaths[i] = mipPaths[i].second;
		}

		return LoadImageMipChainFromFiles(sortedMipPaths, colorSpace);
	}

	constexpr std::array<std::string_view, 6> cubemapFaceNames = {"px", "nx", "py", "ny", "pz", "nz"};

	static int FaceIndexFromName(std::string_view name)
	{
		for (int i = 0; i < 6; i++)
		{
			if (cubemapFaceNames[i] == name) return i;
		}
		return -1;
	}

	std::expected<Cubemap, Utils::Error> LoadCubemapFromFiles(const std::array<std::filesystem::path, 6>& facePaths, Graphics::ColorSpace colorSpace)
	{
		Cubemap result{};
		bool initialized = false;

		for (size_t i = 0; i < 6; i++)
		{
			const auto& path = facePaths[i];

			auto imageResult = LoadImage(path, colorSpace);
			if (!imageResult)
			{
				return std::unexpected(Utils::Error(
					"Failed to load cubemap face {}, error: {}, file path: {}",
					std::string(cubemapFaceNames[i]),
					imageResult.error().Message(),
					path.string()));
			}

			Image img = std::move(imageResult.value());

			if (img.width != img.height)
			{
				return std::unexpected(Utils::Error(
					"Cubemap face {} is not square, got {}x{}, file path: {}",
					std::string(cubemapFaceNames[i]),
					img.width,
					img.height,
					path.string()));
			}

			if (!initialized)
			{
				result.size = img.width;
				result.format = img.format;
				initialized = true;
			}
			else
			{
				if (img.width != result.size || img.height != result.size)
				{
					return std::unexpected(Utils::Error(
						"Cubemap face {} has invalid dimensions, expected {}x{}, but got {}x{}, file path: {}",
						std::string(cubemapFaceNames[i]),
						result.size,
						result.size,
						img.width,
						img.height,
						path.string()));
				}

				if (img.format != result.format)
				{
					return std::unexpected(Utils::Error(
						"Cubemap face {} pixel format mismatch, file path: {}",
						std::string(cubemapFaceNames[i]),
						path.string()));
				}
			}

			result.faces[i] = std::move(img.data);
		}

		return result;
	}

	std::expected<Cubemap, Utils::Error> LoadCubemapFromFolder(const std::filesystem::path& folderPath, Graphics::ColorSpace colorSpace, const std::string& prefix)
	{
		if (!std::filesystem::exists(folderPath))
			return std::unexpected(Utils::Error("Path {} does not exist", folderPath.string()));

		if (!std::filesystem::is_directory(folderPath))
			return std::unexpected(Utils::Error("Path {} is not a directory", folderPath.string()));

		std::unordered_map<std::string, std::filesystem::path> stemToPath;
		stemToPath.reserve(64);

		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(folderPath))
		{
			if (!entry.is_regular_file())
				continue;

			const auto p = entry.path();
			if (p.extension().empty())
				continue;

			stemToPath.emplace(p.stem().string(), p);
		}

		std::string extension;
		std::array<std::filesystem::path, 6> facePaths{};

		for (size_t i = 0; i < 6; i++)
		{
			const std::string expectedStem = prefix + std::string(cubemapFaceNames[i]);

			auto it = stemToPath.find(expectedStem);
			if (it == stemToPath.end())
			{
				return std::unexpected(Utils::Error(
					"Missing cubemap face file {}, expected stem {}, folder path: {}",
					std::string(cubemapFaceNames[i]),
					expectedStem,
					folderPath.string()));
			}

			const auto& p = it->second;
			const auto ext = p.extension().string();

			if (extension.empty())
				extension = ext;
			else if (ext != extension)
			{
				return std::unexpected(Utils::Error(
					"Inconsistent cubemap face extensions, expected {}, but got {}, file path: {}",
					extension,
					ext,
					p.string()));
			}

			facePaths[i] = p;
		}

		return LoadCubemapFromFiles(facePaths, colorSpace);
	}

	std::expected<CubemapMipChain, Utils::Error> LoadCubemapMipChainFromFolder(const std::filesystem::path& folderPath, Graphics::ColorSpace colorSpace)
	{
		if (!std::filesystem::exists(folderPath))
			return std::unexpected(Utils::Error("Path {} does not exist", folderPath.string()));

		if (!std::filesystem::is_directory(folderPath))
			return std::unexpected(Utils::Error("Path {} is not a directory", folderPath.string()));

		struct LevelEntry
		{
			std::array<std::filesystem::path, 6> faces{};
			std::array<bool, 6> present{};
		};

		std::unordered_map<uint32_t, LevelEntry> levels;
		std::string extension;

		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(folderPath))
		{
			if (!entry.is_regular_file())
				continue;

			const auto p = entry.path();
			if (p.extension().empty())
				continue;

			const std::string ext = p.extension().string();
			if (extension.empty())
				extension = ext;
			else if (ext != extension)
			{
				return std::unexpected(Utils::Error(
					"Inconsistent file extensions in cubemap mip chain folder, expected {}, but found {}, file path: {}",
					extension,
					ext,
					p.string()));
			}

			const std::string stem = p.stem().string();
			if (stem.size() < 5 || stem[0] != 'm')
				continue;

			const size_t underscorePos = stem.find('_');
			if (underscorePos == std::string::npos || underscorePos < 2)
				continue;

			const std::string_view levelStr(stem.data() + 1, underscorePos - 1);
			if (!Utils::Text::IsNumeric(levelStr))
				continue;

			const uint32_t level = static_cast<uint32_t>(std::stoul(std::string(levelStr)));
			const std::string_view faceName(stem.data() + underscorePos + 1, stem.size() - (underscorePos + 1));

			const int faceIndex = FaceIndexFromName(faceName);
			if (faceIndex < 0)
				continue;

			auto& e = levels[level];
			if (e.present[faceIndex])
			{
				return std::unexpected(Utils::Error(
					"Duplicate cubemap mip face m{}_{} in folder, file path: {}",
					level,
					std::string(faceName),
					p.string()));
			}

			e.faces[faceIndex] = p;
			e.present[faceIndex] = true;
		}

		if (levels.empty())
		{
			return std::unexpected(Utils::Error(
				"No cubemap mip files found in folder, expected m0_px..mN_nz, folder path: {}",
				folderPath.string()));
		}

		std::vector<uint32_t> levelIndices;
		levelIndices.reserve(levels.size());
		for (const auto& kv : levels)
			levelIndices.push_back(kv.first);

		std::sort(levelIndices.begin(), levelIndices.end());

		const bool contiguous = std::ranges::equal(
			levelIndices,
			std::views::iota(uint32_t{0}, static_cast<uint32_t>(levelIndices.size())));

		if (!contiguous)
		{
			return std::unexpected(Utils::Error(
				"Invalid cubemap mip chain folder structure, expected levels m0..mN with no gaps, folder path: {}",
				folderPath.string()));
		}

		for (uint32_t level : levelIndices)
		{
			const auto& e = levels[level];
			for (int f = 0; f < 6; f++)
			{
				if (!e.present[f])
				{
					return std::unexpected(Utils::Error(
						"Missing cubemap mip face m{}_{} in folder, folder path: {}",
						level,
						std::string(cubemapFaceNames[f]),
						folderPath.string()));
				}
			}
		}

		CubemapMipChain chain{};
		uint32_t expectedSize = 1;

		for (size_t i = 0; i < levelIndices.size(); i++)
		{
			const uint32_t level = levelIndices[i];
			const auto& e = levels[level];

			auto cubemapResult = LoadCubemapFromFiles(e.faces, colorSpace);
			if (!cubemapResult)
			{
				return std::unexpected(Utils::Error(
					"Failed to load cubemap mip level {}, error: {}, folder path: {}",
					level,
					cubemapResult.error().Message(),
					folderPath.string()));
			}

			Cubemap cm = std::move(cubemapResult.value());

			if (i == 0)
			{
				chain.format = cm.format;
				expectedSize = cm.size;
			}
			else
			{
				expectedSize = std::max(1u, expectedSize / 2);

				if (cm.size != expectedSize)
				{
					return std::unexpected(Utils::Error(
						"Invalid cubemap mip level {}, expected size {}, but got {}, folder path: {}",
						level,
						expectedSize,
						cm.size,
						folderPath.string()));
				}

				if (cm.format != chain.format)
				{
					return std::unexpected(Utils::Error(
						"Invalid cubemap mip level {}, pixel format mismatch, folder path: {}",
						level,
						folderPath.string()));
				}
			}

			CubemapMip mip{};
			mip.size = cm.size;
			mip.faces = std::move(cm.faces);
			chain.mipMaps.push_back(std::move(mip));
		}

		return chain;
	}
}