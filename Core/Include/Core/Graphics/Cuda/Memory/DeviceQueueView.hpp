#pragma once
#include <cstddef>
#include "CounterView.hpp"

namespace Core::Graphics::Cuda::Memory
{
    template<typename T>
    struct DeviceQueueView
    {
        T* data = nullptr;
        size_t capacity = 0;
        CounterView counter;
    };
}