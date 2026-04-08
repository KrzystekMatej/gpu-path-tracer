#include <Core/Import/SceneLoader.hpp>
#include <spdlog/spdlog.h>
#include <Core/Utils/Hash.hpp>
#include <Core/Utils/Yaml.hpp>

namespace Core::Import
{
	std::expected<Scene, Utils::Error> LoadScene(const std::filesystem::path& path)
	{
		try
		{
			Scene scene;
			YAML::Node root = YAML::LoadFile(path.string());
			YAML::Node scriptsNode = root["scripts"];

			if (scriptsNode)
			{

				for (const auto& script : scriptsNode)
				{
					YAML::Node phaseNode = script["phase"];
					YAML::Node nameNode = script["name"];

					if (!nameNode)
						return std::unexpected(Utils::Error("Script is missing 'name' field"));
					if (!phaseNode)
						return std::unexpected(Utils::Error("Script is missing 'phase' field"));

					Scripts::Phase phase;
					std::string phaseStr = phaseNode.as<std::string>();

					if (phaseStr == "Awake")
					{
						phase = Scripts::Phase::Awake;
					}
					else if (phaseStr == "Update")
					{
						phase = Scripts::Phase::Update;
					}
					else
					{
						return std::unexpected(Utils::Error("Invalid script phase '{}'", phaseNode.as<std::string>()));
					}

					scene.scripts.emplace_back(Utils::Hasher::MakeId(nameNode.as<std::string>()), phase);
				}
			}

			auto sceneResult = Utils::Yaml::GetSequence(root, "scene");

			if (!sceneResult)
				return std::unexpected(Utils::Error(std::move(sceneResult).error()));

			scene.sceneRoot = sceneResult.value();
			return scene;
		}
		catch (const std::exception& e)
		{
			return std::unexpected(Utils::Error("Failed to load scene: {}", std::string(e.what())));
		}
	}
}