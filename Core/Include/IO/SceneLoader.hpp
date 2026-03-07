#pragma once
#include <expected>
#include <filesystem>
#include "IO/Scene.hpp"
#include "Utils/Error/Error.hpp"

namespace Core::IO
{
	std::expected<Scene, Utils::Error> LoadScene(const std::filesystem::path& path);
}