#pragma once
#include <expected>
#include <unordered_map>
#include <Core/Ecs/SceneNodes/Builder.hpp>
#include <Core/Utils/Error.hpp>
#include <Core/Utils/Guid.hpp>
#include <Core/Utils/Hash.hpp>

namespace Core::Ecs::SceneNodes
{
	class BuilderRegistry 
	{
	public:
		std::expected<std::reference_wrapper<const Builder>, Utils::Error> Get(Utils::Guid id) const
		{
			auto it = m_Builders.find(id);
			if (it == m_Builders.end())
				return std::unexpected(Utils::Error("No builder with id '{}' found!", id));
			return *it->second;
		}

		std::expected<Utils::Guid, Utils::Error> Register(const std::string& name, std::unique_ptr<Builder> builder)
		{
			Utils::Guid id = Utils::Hasher::MakeId(name);
			auto [it, inserted] = m_Builders.try_emplace(id, std::move(builder));
			if (!inserted)
				return std::unexpected(Utils::Error("Builder with name '{}' is already registered!", name));
			return id;
		}

		void RegisterCoreBuilders();
	private:
		std::unordered_map<Utils::Guid, std::unique_ptr<Builder>> m_Builders;
	};
}