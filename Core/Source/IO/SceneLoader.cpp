#include "IO/SceneLoader.hpp"
#include <spdlog/spdlog.h>
#include "Utils/Hash.hpp"

namespace Core::IO
{
	std::expected<Scene, Utils::Error> LoadScene(const std::filesystem::path& path)
	{
		try
		{
			Scene scene;
			YAML::Node root = YAML::LoadFile(path.string());

			if (root["scripts"])
			{
				YAML::Node scripts = root["scripts"];

				for (const auto& script : scripts)
				{
					if (!script["phase"])
						return std::unexpected(Utils::Error("Script is missing 'phase' field"));
					if (!script["name"])
						return std::unexpected(Utils::Error("Script is missing 'name' field"));

					if (script["phase"].as<std::string>() == "Awake")
						scene.scripts.emplace_back(Utils::Hasher::MakeId(script["name"].as<std::string>()), Scripts::Phase::Awake);
					if (script["phase"].as<std::string>() == "Update")
						scene.scripts.emplace_back(Utils::Hasher::MakeId(script["name"].as<std::string>()), Scripts::Phase::Update);
				}
			}

			if (root["scene"])
			{
				scene.sceneRoot = root["scene"];
				return scene;
			}

			spdlog::warn("Scene in file '{}' is empty.", path.string());

			scene.sceneRoot = YAML::Node();
			return scene;
		}
		catch (const std::exception& e)
		{
			return std::unexpected(Utils::Error("Failed to load scene: {}", std::string(e.what())));
		}
	}
}