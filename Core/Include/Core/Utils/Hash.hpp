#pragma once
#include <cstdint>
#include <span>
#include <string>
#include <type_traits>
#include <Core/External/XxHash.hpp>
#include <Core/Utils/Guid.hpp>

namespace Core::Utils
{
    template <typename T>
    struct IsSpan : std::false_type {};

    template <typename T, std::size_t Extent>
    struct IsSpan<std::span<T, Extent>> : std::true_type {};

    template <typename T>
    concept Span = IsSpan<std::remove_cvref_t<T>>::value;


    class Hasher
    {
    public:
        Hasher()
        {
            XXH3_64bits_reset(&m_State);
        }

        void Reset()
        {
            XXH3_64bits_reset(&m_State);
        }

		void Add(std::string_view str)
		{
            uint64_t length = static_cast<uint64_t>(str.size());
            Add(length);
			Add(std::as_bytes(std::span(str.data(), str.size())));
		}

        template <typename T, std::size_t Extent>
        requires std::is_trivially_copyable_v<T>
        void Add(std::span<T, Extent> data)
        {
            const uint64_t size = static_cast<uint64_t>(data.size());
            Add(size);

            auto bytes = std::as_bytes(data);
            XXH3_64bits_update(&m_State, bytes.data(), bytes.size());
        }

        template <typename T>
        requires (
            std::is_trivially_copyable_v<T> &&
            std::has_unique_object_representations_v<T> &&
            !Span<T>
        )
		void Add(const T& value)
		{
            static_assert(!std::is_pointer_v<T>);
			XXH3_64bits_update(&m_State, &value, sizeof(T));
		}
        
		template <typename T>
        static Utils::Guid MakeId(const T& value)
        {
            Hasher hasher;
			hasher.Add(value);
            return hasher.Digest();
        }

        Utils::Guid Digest() const
        {
            return XXH3_64bits_digest(&m_State);
        }

    private:
        XXH3_state_t m_State;
    };
}
