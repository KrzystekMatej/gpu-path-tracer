#include <Core/Project/Descriptor.hpp>
#include <Core/Utils/Path.hpp>

namespace Core::Project
{
	std::expected<Descriptor, Utils::Error> Descriptor::Create(const std::filesystem::path& configPath)
	{
		auto configResult = Config::LoadFromYAML(configPath);
		if (!configResult)
			return std::unexpected(Utils::Error(std::move(configResult).error()));

		Descriptor descriptor(configPath.parent_path(), std::move(configResult).value());
		auto normalizedContent = Utils::Path::NormalizeRelativePath(descriptor.m_Config.contentDirectory);
		if (!normalizedContent)
			return std::unexpected(Utils::Error(std::move(normalizedContent).error()));
		descriptor.m_Config.contentDirectory = std::move(normalizedContent).value();
		if (!std::filesystem::is_directory(descriptor.GetRootPath()))
			return std::unexpected(Utils::Error("Project root directory '{}' does not exist!", descriptor.GetRootPath().string()));
		if (!std::filesystem::is_directory(descriptor.GetContentPath()))
			return std::unexpected(Utils::Error("Project asset directory '{}' does not exist!", descriptor.GetContentPath().string()));
		return descriptor;
	}
}