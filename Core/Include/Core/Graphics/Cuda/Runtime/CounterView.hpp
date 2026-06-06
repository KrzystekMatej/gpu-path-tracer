#pragma once
#include <cstdint>
#include <cuda_runtime.h>

namespace Core::Graphics::Cuda::Runtime
{
    
    template<typename T>
    class CounterView
    {
    public:
        CounterView() = default;
        CounterView(T* value)
            : m_Value(value) {}
        
        __device__ __forceinline__ T Get() const
        {
            return *m_Value;
        }

        __device__ __forceinline__ void Set(T value) const
        {
            *m_Value = value;
        }

        __device__ __forceinline__ T Increment() const
        {
#if defined(__CUDA_ARCH__)
            return atomicAdd(m_Value, 1u);
#else
            return 0;
#endif
        }

        __device__ __forceinline__ T Decrement() const
        {
#if defined(__CUDA_ARCH__)
            return atomicSub(m_Value, 1u);
#else
            return 0;
#endif
        }

        __device__ __forceinline__ T Add(T value) const
        {
#if defined(__CUDA_ARCH__)
            return atomicAdd(m_Value, value);
#else
            return 0;
#endif
        }

        __device__ __forceinline__ T Sub(T value) const
        {
#if defined(__CUDA_ARCH__)
            return atomicSub(m_Value, value);
#else
            return 0;
#endif
        }
    private:
        T* __restrict__ m_Value = nullptr;
    };
}