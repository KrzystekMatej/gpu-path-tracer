#pragma once
#include <vector>
#include <yaml-cpp/yaml.h>
#include <expected>
#include "Utils/Error/Error.hpp"
#include "Scripts/Binding.hpp"

namespace Core::IO
{
	struct Scene
	{
		YAML::Node sceneRoot;
		std::vector<Scripts::Binding> scripts;
	private:
		Scene() = default;
		friend std::expected<Scene, Utils::Error> LoadScene(const std::filesystem::path& path);
	};
}