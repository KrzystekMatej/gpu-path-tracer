#pragma once
#include <filesystem>
#include <expected>
#include "Utils/Error/Error.hpp"
#include "IO/Model.hpp"

namespace Core::IO
{
	std::expected<ParsedModel, Utils::Error> LoadObj(const std::filesystem::path& path);
}