#pragma once
#include <filesystem>
#include <expected>
#include <string>
#include "Utils/Error/Error.hpp"

namespace Core::Utils::Path
{
	struct ResolvedPath
	{
		std::filesystem::path absolute;
		std::filesystem::path relative;
	};

	std::expected<std::filesystem::path, Error> NormalizeRelativePath(const std::filesystem::path& path);
	std::expected<ResolvedPath, Utils::Error> ResolvePath(const std::filesystem::path& path, const std::filesystem::path& root);
}

