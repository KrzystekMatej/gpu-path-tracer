#include "Scripts/Catalog.hpp"
#include "Utils/Hash.hpp"


namespace Core::Scripts
{
	std::expected<Callback, Utils::Error> Catalog::Get(Utils::Guid id) const
	{
		auto it = m_Scripts.find(id);
		if (it == m_Scripts.end())
			return std::unexpected(Utils::Error("No script with id '{}' found!", id));
		return it->second;
	}

	std::expected<Utils::Guid, Utils::Error> Catalog::Register(const std::string& name, Callback script)
	{
		Utils::Guid id = Utils::Hasher::MakeId(name);
		auto [it, inserted] = m_Scripts.try_emplace(id, std::move(script));
		if (!inserted)
			return std::unexpected(Utils::Error("Script with name '{}' is already registered!", name));
		return id;
	}
}