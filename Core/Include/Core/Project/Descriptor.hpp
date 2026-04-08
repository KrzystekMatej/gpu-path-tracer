#pragma once
#include <expected>
#include <Core/Project/Config.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Project
{
	class Descriptor
	{
	public:
		Descriptor(std::filesystem::path rootDirectory,  Config config)
			: m_RootPath(std::move(rootDirectory)), m_Config(std::move(config)) {}

		static std::expected<Descriptor, Utils::Error> Create(const std::filesystem::path& configPath);

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

		const Config& GetConfig() const { return m_Config; }
	private:
		std::filesystem::path m_RootPath;
		Config m_Config;
	};
}
