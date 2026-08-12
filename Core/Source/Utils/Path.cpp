#include <Core/Utils/Path.hpp>

namespace Core::Utils::Path
{
	bool IsSubpath(const std::filesystem::path& base, const std::filesystem::path& path)
	{
		std::filesystem::path normalizedBase = base.lexically_normal();
		std::filesystem::path normalizedPath = path.lexically_normal();

		auto baseIt = normalizedBase.begin();
		auto pathIt = normalizedPath.begin();

		for (; baseIt != normalizedBase.end() && pathIt != normalizedPath.end(); ++baseIt, ++pathIt)
		{
			if (*baseIt != *pathIt)
				return false;
		}

		return baseIt == normalizedBase.end();
	}

	std::expected<std::filesystem::path, Error> NormalizeRelativePath(const std::filesystem::path& path)
	{
		if (path.empty() || path.is_absolute())
			return std::unexpected(Error("Absolute paths are not allowed"));

		std::filesystem::path relative = path.lexically_normal();

		auto it = relative.begin();
		if (it != relative.end() && *it == "..")
			return std::unexpected(Error("Path escapes root"));

		return relative;
	}

	std::expected<ResolvedPath, Utils::Error> ResolvePath(const std::filesystem::path& path, const std::filesystem::path& root)
	{
		CORE_TRY(relativePath, NormalizeRelativePath(path));
		std::filesystem::path absolute = root / relativePath;
		return ResolvedPath{ .absolute = std::move(absolute), .relative = std::move(relativePath) };
	}

	std::expected<void, Core::Utils::Error> EnsureDirectoryExists(const std::filesystem::path& directory)
	{
		if (directory.empty())
			return {};

		std::error_code errorCode;

		const bool exists = std::filesystem::exists(directory, errorCode);
		if (errorCode)
			return std::unexpected(Core::Utils::Error("Failed to check output directory '{}': {}", directory.string(), errorCode.message()));

		if (exists)
		{
			const bool isDirectory = std::filesystem::is_directory(directory, errorCode);
			if (errorCode)
				return std::unexpected(Core::Utils::Error("Failed to inspect output directory '{}': {}", directory.string(), errorCode.message()));

			if (!isDirectory)
				return std::unexpected(Core::Utils::Error("Output path '{}' exists but is not a directory", directory.string()));
			
			return {};
		}

		if (!std::filesystem::create_directories(directory, errorCode) && errorCode)
			return std::unexpected(Core::Utils::Error("Failed to create output directory '{}': {}", directory.string(), errorCode.message()));

		return {};
	}
}