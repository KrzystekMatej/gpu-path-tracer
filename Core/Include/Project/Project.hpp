#pragma once
#include <expected>
#include "Project/ProjectConfig.hpp"
#include "Utils/Error/Error.hpp"

namespace Core
{
	class Project
	{
	public:
		explicit Project(std::filesystem::path rootDirectory,  ProjectConfig config)
			: m_RootPath(std::move(rootDirectory)), m_Config(std::move(config)) {}

		static std::expected<Project, Utils::Error> Create(const std::filesystem::path& configFilePath);

		const std::filesystem::path& GetRootPath() const
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

		std::filesystem::path GetBackgroundsPath() const
		{
			return m_RootPath / m_Config.BackgroundDirectory;
		}

		const ProjectConfig& GetConfig() const { return m_Config; }

	private:
		std::filesystem::path m_RootPath;
		ProjectConfig m_Config;
	};
}
