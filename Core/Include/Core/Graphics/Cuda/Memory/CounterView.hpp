#pragma once
#include <cstdint>

namespace Core::Graphics::Cuda::Memory
{
    struct CounterView
    {
        uint32_t* data = nullptr;
    };
}