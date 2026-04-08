#include <Core/Utils/Yaml.hpp>

namespace Core::Utils::Yaml
{
	std::expected<YAML::Node, Error> LoadFile(const std::filesystem::path& path)
	{
		try
		{
			YAML::Node node = YAML::LoadFile(path.string());
			return node;
		}
		catch (const std::exception& e)
		{
			return std::unexpected(Error("Failed to load YAML file '{}': {}", path.string(), std::string(e.what())));
		}
	}

	std::expected<YAML::Node, Error> GetScalar(const YAML::Node& node, const char* key)
	{
		YAML::Node scalarNode = node[key];
		if (!scalarNode)
			return std::unexpected(Error("Missing required '{}' field", key));
		if (!scalarNode.IsScalar())
			return std::unexpected(Error("Expected '{}' field to be a scalar", key));
		return scalarNode;
	}

	std::expected<YAML::Node, Error> GetSequence(const YAML::Node& node, const char* key)
	{
		YAML::Node seqNode = node[key];
		if (!seqNode)
			return std::unexpected(Error("Missing required '{}' field", key));
		if (!seqNode.IsSequence())
			return std::unexpected(Error("Expected '{}' field to be a sequence", key));
		return seqNode;
	}

	std::expected<YAML::Node, Error> GetSizedSequence(const YAML::Node& node, const char* key, std::size_t expectedSize)
	{
		YAML::Node seqNode = node[key];
		if (!seqNode)
			return std::unexpected(Error("Missing required '{}' field", key));
		if (!seqNode.IsSequence())
			return std::unexpected(Error("Expected '{}' field to be a sequence", key));
		if (seqNode.size() != expectedSize)
			return std::unexpected(Error("Expected '{}' field to be a sequence of {} elements", key, expectedSize));
		return seqNode;
	}

	std::expected<YAML::Node, Error> GetMap(const YAML::Node& node, const char* key)
	{
		YAML::Node mapNode = node[key];
		if (!mapNode)
			return std::unexpected(Error("Missing required '{}' field", key));
		if (!mapNode.IsMap())
			return std::unexpected(Error("Expected '{}' field to be a map", key));
		return mapNode;
	}

	std::expected<std::string, Error> GetString(const YAML::Node& node, const char* key)
	{
		auto scalarResult = GetScalar(node, key);
		if (!scalarResult)
			return std::unexpected(std::move(scalarResult).error());

		YAML::Node scalarNode = scalarResult.value();
		return scalarNode.as<std::string>();
	}

	std::expected<float, Error> GetFloat(const YAML::Node& node, const char* key)
	{
		auto scalarResult = GetScalar(node, key);
		if (!scalarResult)
			return std::unexpected(std::move(scalarResult).error());

		YAML::Node scalarNode = scalarResult.value();

		try
		{
			float value = scalarNode.as<float>();
			return value;
		}
		catch (const std::exception& e)
		{
			return std::unexpected(Error("Failed to parse '{}' field as float: {}", key, std::string(e.what())));
		}
	}

	std::expected<glm::vec3, Error> GetVec3(const YAML::Node& node, const char* key)
	{
		auto sequenceResult = GetSizedSequence(node, key, 3);
		if (!sequenceResult)
			return std::unexpected(std::move(sequenceResult).error());

		YAML::Node vecNode = sequenceResult.value();

		try
		{
			glm::vec3 vec;
			vec.x = vecNode[0].as<float>();
			vec.y = vecNode[1].as<float>();
			vec.z = vecNode[2].as<float>();
			return vec;
		}
		catch (const std::exception& e)
		{
			return std::unexpected(Error("Failed to parse '{}' field as vec3: {}", key, std::string(e.what())));
		}
	}

	std::expected<glm::vec4, Error> GetVec4(const YAML::Node& node, const char* key)
	{
		auto sequenceResult = GetSizedSequence(node, key, 4);
		if (!sequenceResult)
			return std::unexpected(std::move(sequenceResult).error());

		YAML::Node vecNode = sequenceResult.value();

		try
		{
			glm::vec4 vec;
			vec.x = vecNode[0].as<float>();
			vec.y = vecNode[1].as<float>();
			vec.z = vecNode[2].as<float>();
			vec.w = vecNode[3].as<float>();
			return vec;
		}
		catch (const std::exception& e)
		{
			return std::unexpected(Error("Failed to parse '{}' field as vec4: {}", key, std::string(e.what())));
		}
	}
}