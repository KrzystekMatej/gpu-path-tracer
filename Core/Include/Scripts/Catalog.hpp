#pragma once
#include <string>
#include <unordered_map>
#include <expected>
#include <variant>
#include "Scripts/Callback.hpp"
#include "Utils/Error/Error.hpp"
#include "Utils/Guid.hpp"

namespace Core::Scripts
{
	class Catalog
	{
	public:
		std::expected<Callback, Utils::Error> Get(Utils::Guid id) const;
		std::expected<Utils::Guid, Utils::Error> Register(const std::string& name, Callback script);
	private:
		std::unordered_map<Utils::Guid, Callback> m_Scripts;
	};
}