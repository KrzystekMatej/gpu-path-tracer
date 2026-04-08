#pragma once
#include <filesystem>
#include <expected>
#include <Core/Utils/Error.hpp>
#include <Core/Import/Model.hpp>

namespace Core::Import
{
	std::expected<ParsedModel, Utils::Error> LoadObj(const std::filesystem::path& path);
}