#pragma once
#include <filesystem>
#include <expected>
#include "Error/Error.hpp"
#include "IO/Model.hpp"

namespace Core::IO
{
	std::expected<ParsedModel, Error> LoadObj(const std::filesystem::path& path);
}