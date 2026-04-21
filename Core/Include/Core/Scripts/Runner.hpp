#pragma once
#include <vector>
#include <optional>
#include <Core/Ecs/Scene.hpp>
#include <Core/Scripts/Catalog.hpp>

namespace Core::Scripts
{
	class Runner
	{
	public:
		void Awake()
		{
			for (const auto& script : m_Awakes)
			{
				script();
			}
		}


		void Update()
		{
			for (const auto& script : m_Updates)
			{
				script();
			}
		}

		std::expected<void, Utils::Error> Bind(const Scripts::Catalog& catalog, const Ecs::Scene& scene);
	private:
		std::vector<Scripts::Callback> m_Awakes;
		std::vector<Scripts::Callback> m_Updates;
	};
}