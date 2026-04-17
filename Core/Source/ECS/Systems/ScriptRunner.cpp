#include <Core/Ecs/Systems/ScriptRunner.hpp>

namespace Core::Ecs::Systems
{
	std::expected<void, Utils::Error> ScriptRunner::Bind(const Scripts::Catalog& catalog, const Ecs::Scene& scene)
	{
		m_Awakes.clear();
		m_Updates.clear();

		for (const auto& binding : scene.GetScriptBindings())
		{
			auto scriptResult = catalog.Get(binding.id);
			if (!scriptResult)
				return std::unexpected(scriptResult.error());

			if (binding.phase == Scripts::Phase::Awake)
			{
				m_Awakes.push_back(scriptResult.value());
			}
			if (binding.phase == Scripts::Phase::Update)
			{
				m_Updates.push_back(scriptResult.value());
			}
		}

		return {};
	}
}