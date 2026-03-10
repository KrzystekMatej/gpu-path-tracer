#pragma once
#include <vector>
#include <optional>
#include "Scripts/Catalog.hpp"

namespace Core::ECS::Systems
{
	class ScriptRunner
	{
	public:
		void Awake(const Context& context)
		{
			for (const auto& script : m_Awakes)
			{
				script(context);
			}
		}


		void Update(const Context& context)
		{
			for (const auto& script : m_Updates)
			{
				script(context);
			}
		}

		std::expected<void, Utils::Error> Bind(const Scripts::Catalog& catalog, const ECS::Scene& scene);
	private:
		std::vector<Scripts::Callback> m_Awakes;
		std::vector<Scripts::Callback> m_Updates;
	};
}