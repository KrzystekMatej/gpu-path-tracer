#pragma once

#include <string_view>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <expected>
#include "Utils/Error/Error.hpp"

namespace Core
{
    struct ProjectConfig
    {
		static constexpr const char* nameKey = "name";
		static constexpr const char* contentKey = "content";
		static constexpr const char* startSceneKey = "start-scene";

        std::string name;
        std::filesystem::path contentDirectory;
        std::filesystem::path startScene;

        static std::expected<ProjectConfig, Utils::Error> LoadFromYAML(const std::filesystem::path& configPath)
        {
            YAML::Node root = YAML::LoadFile(configPath.string());

			YAML::Node nameNode = root[nameKey];
			YAML::Node contentNode = root[contentKey];
			YAML::Node sceneNode = root[startSceneKey];

            if (!nameNode)
				return std::unexpected(Utils::Error("Project config is missing required '{}' field", nameKey));
            if (!contentNode)
                return std::unexpected(Utils::Error("Project config is missing required '{}' field", contentKey));
            if (!sceneNode)
                return std::unexpected(Utils::Error("Project config is missing required '{}' field", startSceneKey));

            return ProjectConfig
            {
                .name = nameNode.as<std::string>(),
                .contentDirectory = contentNode.as<std::string>(),
                .startScene = sceneNode.as<std::string>()
            };
        }
    };
}
