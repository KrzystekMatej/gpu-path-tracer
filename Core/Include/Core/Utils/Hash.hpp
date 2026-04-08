#pragma once
#include <cstdint>
#include <span>
#include <string>
#include <type_traits>
#include <Core/External/XxHash.hpp>
#include <Core/Utils/Guid.hpp>

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

		void Update(std::string_view str)
		{
			Update(std::as_bytes(std::span(str.data(), str.size())));
		}

        void Update(const std::span<const std::byte>& data)
        {
            XXH3_64bits_update(&m_State, data.data(), data.size());
        }

		template <typename T>
		requires (std::is_trivially_copyable_v<T> &&
				  std::has_unique_object_representations_v<T>)
		void Update(const T& value)
		{
            static_assert(!std::is_pointer_v<T>);
			XXH3_64bits_update(&m_State, &value, sizeof(T));
		}
        
		template <typename T>
        static Utils::Guid MakeId(const T& value)
        {
            Hasher hasher;
			hasher.Update(value);
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
