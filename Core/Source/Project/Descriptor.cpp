#include <Core/Project/Descriptor.hpp>
#include <Core/Utils/Path.hpp>

namespace Core::Project
{
	std::expected<Descriptor, Utils::Error> Descriptor::Create(const std::filesystem::path& configPath)
	{
		std::filesystem::path absoluteConfigPath;

		try
		{
			absoluteConfigPath = std::filesystem::absolute(configPath).lexically_normal();
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			return std::unexpected(Utils::Error(
				"Failed to resolve project config path '{}': {}",
				configPath.string(),
				e.what()
			));
		}

		auto configResult = Config::LoadFromYAML(absoluteConfigPath);
		if (!configResult)
			return std::unexpected(Utils::Error(std::move(configResult).error()));

		Descriptor descriptor(absoluteConfigPath.parent_path(), std::move(configResult).value());
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