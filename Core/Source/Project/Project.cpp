#include "Project/Project.hpp"

namespace Core
{
	std::expected<Project, Utils::Error> Project::Create(const std::filesystem::path& configPath)
	{
		ProjectConfig config = ProjectConfig::LoadFromYAML(configPath);

		Project project(configPath.parent_path(), config);

		if (!std::filesystem::is_directory(project.GetRootPath()))
			return std::unexpected(Utils::Error("Project root directory '{}' does not exist!", project.GetRootPath().string()));

		if (!std::filesystem::is_directory(project.GetAssetsPath()))
			return std::unexpected(Utils::Error("Project asset directory '{}' does not exist!", project.GetAssetsPath().string()));

		if (!std::filesystem::is_directory(project.GetScenesPath()))
			return std::unexpected(Utils::Error("Project scene directory '{}' does not exist!", project.GetScenesPath().string()));

		if (!std::filesystem::is_directory(project.GetShadersPath()))
			return std::unexpected(Utils::Error("Project shader directory '{}' does not exist!", project.GetShadersPath().string()));

		if (!std::filesystem::is_directory(project.GetModelsPath()))
			return std::unexpected(Utils::Error("Project model directory '{}' does not exist!", project.GetModelsPath().string()));

		if (!std::filesystem::is_directory(project.GetBackgroundsPath()))
			return std::unexpected(Utils::Error("Project background directory '{}' does not exist!", project.GetBackgroundsPath().string()));

		return project;
	}
}