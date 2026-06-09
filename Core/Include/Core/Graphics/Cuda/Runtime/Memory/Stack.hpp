#pragma once
#include <cstdint>
#include <cuda_runtime.h>

namespace Core::Graphics::Cuda::Runtime
{
    template<typename T, size_t Capacity>
    class Stack
    {
    public:
        __forceinline__ __device__ void Push(T value)
        {
            assert(pointer < Capacity);
            data[pointer++] = value;
        }

        __forceinline__ __device__ T Peek() const
        {
            assert(pointer > 0);
            return data[pointer - 1];
        }

        __forceinline__ __device__ T Pop()
        {
            assert(pointer > 0);
            return data[--pointer];
        }

        __forceinline__ __device__ bool IsEmpty() const { return pointer == 0; }
        __forceinline__ __device__ bool IsFull() const { return pointer == Capacity; }
        __forceinline__ __device__ void Clear() { pointer = 0; }

    private:
        uint32_t pointer = 0;
        T data[Capacity];
    };
}