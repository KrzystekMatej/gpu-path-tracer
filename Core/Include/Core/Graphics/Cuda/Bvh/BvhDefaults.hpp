#pragma once
#include <cstdint>

namespace Core::Graphics::Cuda
{
    class BvhDefaults
    {
    public:
        constexpr static uint32_t MaxDepth = 64;
    };
}