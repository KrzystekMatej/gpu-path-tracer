#pragma once
#include <expected>
#include <unordered_map>
#include "ECS/Resolvers/Resolver.hpp"
#include "Utils/Error/Error.hpp"
#include "Utils/Guid.hpp"
#include "Utils/Hash.hpp"

namespace Core::ECS
{
	class SceneResolverRegistry 
	{
	public:
		std::expected<std::reference_wrapper<const SceneNodeResolver>, Utils::Error> GetResolver(Utils::Guid id) const
		{
			auto it = m_Resolvers.find(id);
			if (it == m_Resolvers.end())
				return std::unexpected(Utils::Error("No resolver with id '{}' found!", id));
			return *it->second;
		}

		std::expected<Utils::Guid, Utils::Error> RegisterResolver(const std::string& name, std::unique_ptr<SceneNodeResolver> resolver)
		{
			Utils::Guid id = Utils::Hasher::MakeId(name);
			auto [it, inserted] = m_Resolvers.try_emplace(id, std::move(resolver));
			if (!inserted)
				return std::unexpected(Utils::Error("Resolver with name '{}' is already registered!", name));
			return id;
		}

		void RegisterCoreResolvers();
	private:
		std::unordered_map<Utils::Guid, std::unique_ptr<SceneNodeResolver>> m_Resolvers;
	};
}