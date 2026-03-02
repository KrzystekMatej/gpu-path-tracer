#pragma once
#include <cstdint>
#include <span>
#include <type_traits>
#include "External/XxHash.hpp"

namespace Core::Utils
{
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

        void Update(std::span<const std::byte> data)
        {
            XXH3_64bits_update(&m_State, data.data(), data.size());
        }

		template <typename T>
		void Update(const T& value)
		{
			static_assert(std::is_trivially_copyable_v<T>);
			static_assert(std::has_unique_object_representations_v<T>);
			XXH3_64bits_update(&m_State, &value, sizeof(T));
		}

        uint64_t Digest() const
        {
            return XXH3_64bits_digest(&m_State);
        }

    private:
        XXH3_state_t m_State;
    };
}
