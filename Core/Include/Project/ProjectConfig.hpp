#pragma once

#include <string>
#include <filesystem>
#include <yaml-cpp/yaml.h>

namespace Core
{
    struct ProjectConfig
    {
        std::string Name = "Untitled";

        std::filesystem::path AssetDirectory = "Assets";
        std::filesystem::path SceneDirectory = "Assets/Scenes";
        std::filesystem::path ShaderDirectory = "Assets/Shaders";
        std::filesystem::path ModelDirectory = "Assets/Models";

        static ProjectConfig LoadFromYAML(const std::filesystem::path& configPath)
        {
            YAML::Node config = YAML::LoadFile(configPath.string());

            ProjectConfig result;
            if (config["name"])
                result.Name = config["name"].as<std::string>();

            if (config["assets"])
                result.AssetDirectory = config["assets"].as<std::string>();

            if (config["scenes"])
                result.SceneDirectory = config["scenes"].as<std::string>();

            if (config["shaders"])
                result.ShaderDirectory = config["shaders"].as<std::string>();

            if (config["models"])
                result.ModelDirectory = config["models"].as<std::string>();

            return result;
        }
    };
}
