#pragma once
#include <expected>
#include "Project/ProjectConfig.hpp"
#include "Utils/Error/Error.hpp"

namespace Core
{
	class Project
	{
	public:
		Project(std::filesystem::path rootDirectory,  ProjectConfig config)
			: m_RootPath(std::move(rootDirectory)), m_Config(std::move(config)) {}

		static std::expected<Project, Utils::Error> Create(const std::filesystem::path& configPath);

		const std::filesystem::path& GetRootPath() const
		{
			return m_RootPath;
		}

		std::filesystem::path GetContentPath() const
		{
			return m_RootPath / m_Config.contentDirectory;
		}

		std::filesystem::path GetStartScenePath() const
		{
			return m_RootPath / m_Config.startScene;
		}

		const ProjectConfig& GetConfig() const { return m_Config; }
	private:
		std::filesystem::path m_RootPath;
		ProjectConfig m_Config;
	};
}
