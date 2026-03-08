#pragma once
#include <expected>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include "External/Glm.hpp"
#include "Utils/Error/Error.hpp"

namespace Core::Utils::Yaml
{
	std::expected<YAML::Node, Error> LoadFile(const std::filesystem::path& path);
	std::expected<YAML::Node, Error> GetScalar(const YAML::Node& node, const char* key);
	std::expected<YAML::Node, Error> GetSequence(const YAML::Node& node, const char* key);
	std::expected<YAML::Node, Error> GetSizedSequence(const YAML::Node& node, const char* key, size_t expectedSize);
	std::expected<YAML::Node, Error> GetMap(const YAML::Node& node, const char* key);
	std::expected<float, Error> GetFloat(const YAML::Node& node, const char* key);
	std::expected<std::string, Error> GetString(const YAML::Node& node, const char* key);
	std::expected<glm::vec3, Error> GetVec3(const YAML::Node& node, const char* key);
	std::expected<glm::vec4, Error> GetVec4(const YAML::Node& node, const char* key);
}