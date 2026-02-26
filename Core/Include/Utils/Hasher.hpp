#pragma once
#include <cstdint>
#include "xxhash.h"

namespace Core::Utils
{
    class Hasher
    {
    public:
        Hasher()
        {
            state = XXH3_createState();
            XXH3_64bits_reset(state);
        }

        ~Hasher()
        {
            XXH3_freeState(state);
        }

        Hasher(const Hasher&) = delete;
        Hasher& operator=(const Hasher&) = delete;

        Hasher(Hasher&& other) noexcept : state(other.state)
        {
            other.state = nullptr;
        }

        Hasher& operator=(Hasher&& other) noexcept
        {
            if (this != &other)
            {
                XXH3_freeState(state);
                state = other.state;
                other.state = nullptr;
            }
            return *this;
        }

        void Reset()
        {
            XXH3_64bits_reset(state);
        }

		void Update(std::span<const std::byte> data)
		{
			XXH3_64bits_update(state, data.data(), data.size());
		}

        template <typename T>
		void Update(const T& value)
		{
			static_assert(std::is_trivially_copyable_v<T>);
			XXH3_64bits_update(state, &value, sizeof(T));
		}

        uint64_t Digest() const
        {
            return XXH3_64bits_digest(state);
        }

    private:
        XXH3_state_t* state;
    };
}
