#pragma once
#include <cstdint>

namespace Core::Scripts
{
    enum class Phase : uint8_t
    {
        Awake,
        Update
    };
}