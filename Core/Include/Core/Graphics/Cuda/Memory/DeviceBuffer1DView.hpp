#pragma once
#include <cstddef>

namespace Core::Graphics::Cuda::Memory
{
    template<typename T>
    struct DeviceBuffer1DView
    {
        T* data = nullptr;
        size_t size = 0;
    };
}