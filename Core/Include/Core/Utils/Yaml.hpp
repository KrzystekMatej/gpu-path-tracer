#pragma once
#include <expected>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <Core/External/Glm.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Utils::Yaml
{
	std::expected<YAML::Node, Error> LoadFile(const std::filesystem::path& path);
	std::expected<YAML::Node, Error> GetScalarNode(const YAML::Node& node, const char* key);
	std::expected<YAML::Node, Error> GetSequence(const YAML::Node& node, const char* key);
	std::expected<YAML::Node, Error> GetSizedSequence(const YAML::Node& node, const char* key, size_t expectedSize);
	std::expected<YAML::Node, Error> GetMap(const YAML::Node& node, const char* key);

	template<typename T>
	std::expected<T, Error> GetValue(const YAML::Node& node, const char* key)
	{
		CORE_TRY(scalarNode, GetScalarNode(node, key));

		try
		{
			T value = scalarNode.as<T>();
			return value;
		}
		catch (const std::exception& e)
		{
			return std::unexpected(Error("Failed to parse '{}' field as {}: {}", key, typeid(T).name(), std::string(e.what())));
		}
	}

	template<>
	std::expected<glm::vec3, Error> GetValue<glm::vec3>(const YAML::Node& node, const char* key);
	template<>
	std::expected<glm::vec4, Error> GetValue<glm::vec4>(const YAML::Node& node, const char* key);
}