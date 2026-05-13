#include <Core/Scripts/Runner.hpp>

namespace Core::Scripts
{
	std::expected<void, Utils::Error> Runner::Bind(const Scripts::Catalog& catalog, const Ecs::Scene& scene)
	{
		m_Awakes.clear();
		m_Updates.clear();

		for (const auto& binding : scene.GetScriptBindings())
		{
			CORE_TRY_CONTEXT(script, catalog.Get(binding.id), "Failed to get script with id {} from catalog", binding.id);

			if (binding.phase == Scripts::Phase::Awake)
				m_Awakes.push_back(script);
			
			if (binding.phase == Scripts::Phase::Update)
				m_Updates.push_back(script);
		}

		return {};
	}
}