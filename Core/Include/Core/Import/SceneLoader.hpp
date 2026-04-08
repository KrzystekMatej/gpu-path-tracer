#pragma once
#include <expected>
#include <filesystem>
#include <Core/Import/Scene.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Import
{
	std::expected<Scene, Utils::Error> LoadScene(const std::filesystem::path& path);
}