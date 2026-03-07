#pragma once
#include <vector>
#include <optional>
#include "Scripts/Functions.hpp"
#include "Scripts/Catalog.hpp"

namespace Core::ECS::Systems
{
	class ScriptRunner
	{
	public:
		void Awake(Scene& scene)
		{
			for (const auto& script : m_Awakes)
			{
				script(scene);
			}
		}


		void Update(Scene& scene, const App::Time& time)
		{
			for (const auto& script : m_Updates)
			{
				script(scene, time);
			}
		}

		std::expected<void, Utils::Error> Bind(const Scripts::Catalog& catalog, const ECS::Scene& scene);
	private:
		std::vector<Scripts::Awake> m_Awakes;
		std::vector<Scripts::Update> m_Updates;
	};
}