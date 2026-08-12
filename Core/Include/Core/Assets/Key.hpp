#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <variant>

#include <Core/Assets/Asset.hpp>
#include <Core/Utils/Guid.hpp>
#include <Core/Utils/Hash.hpp>
#include <Core/Utils/Text.hpp>

namespace Core::Assets
{
    template<Asset T>
    class KeyBuilder;

    template<Asset T>
    class Key
    {
    public:
        Utils::Guid GetId() const { return m_Id; }
        std::string_view GetDebugName() const { return m_DebugName.View(); }

    private:
        Key(Utils::Guid id, Utils::Text::StringLiteral debugName)
            : m_Id(id), m_DebugName(debugName) {}

        friend class KeyBuilder<T>;

        Utils::Guid m_Id;
        Utils::Text::StringLiteral m_DebugName;
    };
    
    template<Asset T>
    class KeyBuilder
    {
    public:
        explicit KeyBuilder()
        {
            Add(static_cast<uint32_t>(T::Type));
        }

		void Add(std::string_view str)
		{
            m_Hasher.Add(str);
		}

        void Add(const std::span<const std::byte>& data)
        {
            m_Hasher.Add(data);
        }

		template <typename T>
		void Add(const T& value)
		{
			m_Hasher.Add(value);
		}

        Key<T> Finish(Utils::Text::StringLiteral debugName)
        {
            return Key<T>(m_Hasher.Digest(), debugName);
        }
    private:
        Utils::Hasher m_Hasher;
    };
}