#include "ECS/Systems/ScriptRunner.hpp"

namespace Core::ECS::Systems
{
	std::expected<void, Utils::Error> ScriptRunner::Bind(const Scripts::Catalog& catalog, const ECS::Scene& scene)
	{
		m_Awakes.clear();
		m_Updates.clear();

		for (const auto& binding : scene.GetScriptBindings())
		{
			if (binding.phase == Scripts::Phase::Awake)
			{
				auto scriptResult = catalog.Get(binding.id);
				if (!scriptResult)
					return std::unexpected(scriptResult.error());

				auto script = scriptResult.value();
				if (!std::holds_alternative<Scripts::Awake>(script))
					return std::unexpected(Utils::Error("Script with id '{}' was bound to an incorrect phase!", binding.id));

				m_Awakes.push_back(std::get<Scripts::Awake>(script));
			}
			if (binding.phase == Scripts::Phase::Update)
			{
				auto scriptResult = catalog.Get(binding.id);
				if (!scriptResult)
					return std::unexpected(scriptResult.error());

				auto script = scriptResult.value();
				if (!std::holds_alternative<Scripts::Update>(script))
					return std::unexpected(Utils::Error("Script with id '{}' was bound to an incorrect phase!", binding.id));

				m_Updates.push_back(std::get<Scripts::Update>(script));
			}
		}

		return {};
	}
}