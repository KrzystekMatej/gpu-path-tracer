#include <Core/Utils/Path.hpp>

namespace Core::Utils::Path
{
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
		auto relativeResult = NormalizeRelativePath(path);
		if (!relativeResult)
			return std::unexpected(std::move(relativeResult).error());
		std::filesystem::path absolute = root / relativeResult.value();
		return ResolvedPath{ .absolute = std::move(absolute), .relative = std::move(relativeResult).value() };
	}
}