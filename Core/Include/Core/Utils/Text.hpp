#pragma once
#include <string>
#include <string_view>

namespace Core::Utils::Text
{
	class StringLiteral
    {
    public:
        consteval StringLiteral(const char* value)
            : m_Value(value)
        {
        }

        consteval StringLiteral(std::string_view value)
            : m_Value(value)
        {
        }

        constexpr std::string_view View() const
        {
            return m_Value;
        }

    private:
        std::string_view m_Value;
    };

	bool IsNumeric(std::string_view str);
}