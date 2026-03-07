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

    inline bool IsOutsideRoot(const std::filesystem::path& relative)
    {
        if (relative.empty() || relative.is_absolute())
            return true;

        auto it = relative.begin();
        return (it != relative.end() && *it == "..");
    }

	inline std::expected<ResolvedPath, Core::Utils::Error> Resolve(
		const std::filesystem::path& input,
		const std::filesystem::path& root)
	{
		std::error_code errorCode;
		std::filesystem::path absolute = input.is_absolute() ? input : (root / input);
		absolute = std::filesystem::weakly_canonical(absolute, errorCode);
		if (errorCode)
			return std::unexpected(Core::Utils::Error("Failed to resolve path: {}", absolute.string()));

		std::filesystem::path relative = absolute.lexically_relative(root).lexically_normal();

		if (relative.empty() || relative.has_root_path())
			return std::unexpected(Core::Utils::Error("Path is not under root: {}", absolute.string()));

		if (auto it = relative.begin(); it != relative.end() && *it == "..")
			return std::unexpected(Core::Utils::Error("Import path escapes root: {}", relative.generic_string()));

		return ResolvedPath{ .absolute = absolute, .relative = relative };
	}
}

