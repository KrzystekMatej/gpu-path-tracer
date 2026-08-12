#pragma once
#include <filesystem>
#include <expected>
#include <Core/Utils/Error.hpp>
#include <Core/Import/Prefabs/ScenePrefab.hpp>

namespace Core::Import
{
	std::expected<ScenePrefab, Utils::Error> LoadObj(const std::filesystem::path& path);
}