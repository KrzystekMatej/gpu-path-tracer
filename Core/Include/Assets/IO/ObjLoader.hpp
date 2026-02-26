#pragma once
#include <filesystem>
#include <expected>
#include "Error/Error.hpp"
#include "Assets/IO/Types.hpp"

namespace Core::Assets::IO
{
	std::expected<ParsedModel, Error> Load(const std::filesystem::path& path);
}