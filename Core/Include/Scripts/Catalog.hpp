#pragma once
#include <string>
#include <unordered_map>
#include <expected>
#include <variant>
#include "Scripts/Functions.hpp"
#include "Utils/Error/Error.hpp"
#include "Utils/Guid.hpp"

namespace Core::Scripts
{
	class Catalog
	{
	public:
		std::expected<std::variant<Awake, Update>, Utils::Error> Get(Utils::Guid id) const;
		std::expected<Utils::Guid, Utils::Error> Register(const std::string& name, std::variant<Awake, Update> script);
	private:
		std::unordered_map<Utils::Guid, std::variant<Awake, Update>> m_Scripts;
	};
}