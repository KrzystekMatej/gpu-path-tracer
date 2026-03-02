#include "Utils/Text.hpp"
#include <algorithm>

namespace Core::Utils::Text
{
	bool IsNumeric(std::string_view str)
	{
		return !str.empty() && std::all_of(str.begin(), str.end(), std::isdigit);
	}
}