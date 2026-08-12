#pragma once
#include <filesystem>
#include <expected>
#include <Core/Utils/Error.hpp>
#include <Core/Import/Prefabs/ScenePrefab.hpp>
#include <Core/Import/Buffers/Buffer.hpp>

namespace Core::Import
{
	std::expected<ScenePrefab, Utils::Error> LoadGlTF(const std::filesystem::path& path);
}