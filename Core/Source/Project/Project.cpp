#include "Project/Project.hpp"
#include "Utils/Path.hpp"

namespace Core
{
	std::expected<Project, Utils::Error> Project::Create(const std::filesystem::path& configPath)
	{
		auto configResult = ProjectConfig::LoadFromYAML(configPath);
		if (!configResult)
			return std::unexpected(Utils::Error(std::move(configResult).error()));

		Project project(configPath.parent_path(), std::move(configResult).value());

		auto normalizedContent = Utils::Path::NormalizeRelativePath(project.m_Config.contentDirectory);
		if (!normalizedContent)
			return std::unexpected(Utils::Error(std::move(normalizedContent).error()));
		project.m_Config.contentDirectory = std::move(normalizedContent).value();

		if (!std::filesystem::is_directory(project.GetRootPath()))
			return std::unexpected(Utils::Error("Project root directory '{}' does not exist!", project.GetRootPath().string()));
		if (!std::filesystem::is_directory(project.GetContentPath()))
			return std::unexpected(Utils::Error("Project asset directory '{}' does not exist!", project.GetContentPath().string()));

		return project;
	}
}