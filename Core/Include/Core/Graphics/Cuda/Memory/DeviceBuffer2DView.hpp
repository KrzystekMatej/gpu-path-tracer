#pragma once
#include <cstddef>

namespace Core::Graphics::Cuda::Memory
{
    template<typename T>
    struct DeviceBuffer2DView
    {
        T* data = nullptr;
        size_t width = 0;
        size_t height = 0;
        size_t pitch = 0;
    };
}