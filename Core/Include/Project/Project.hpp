#pragma once

#include "Project/ProjectConfig.hpp"

namespace Core
{
	class Project
	{
	public:
		explicit Project(std::filesystem::path rootDirectory,  ProjectConfig config)
			: m_RootPath(std::move(rootDirectory)), m_Config(std::move(config)) {}

		static Project Load(const std::filesystem::path& configFilePath)
		{
			ProjectConfig config = ProjectConfig::LoadFromYAML(configFilePath);
			return Project(configFilePath.parent_path(), config);
		}

		const std::filesystem::path& GetRootDirectory() const
		{
			return m_RootPath;
		}

		std::filesystem::path GetAssetsPath() const
		{
			return m_RootPath / m_Config.AssetDirectory;
		}

		std::filesystem::path GetScenesPath() const
		{
			return m_RootPath / m_Config.SceneDirectory;
		}

		std::filesystem::path GetShadersPath() const
		{
			return m_RootPath / m_Config.ShaderDirectory;
		}

		std::filesystem::path GetModelsPath() const
		{
			return m_RootPath / m_Config.ModelDirectory;
		}

		const ProjectConfig& GetConfig() const { return m_Config; }

	private:
		std::filesystem::path m_RootPath;
		ProjectConfig m_Config;
	};
}
