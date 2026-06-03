#pragma once
#include <cstddef>
#include <cuda_runtime.h>

namespace Core::Graphics::Cuda::Memory
{
    template<typename T>
    class DeviceBuffer1DView
    {
    public:
        DeviceBuffer1DView(T* data, uint32_t size)
            : m_Data(data), m_Size(size) {}

        __device__ __forceinline__ T& At(uint32_t index)
        {
            return m_Data[index];
        }

        __device__ __forceinline__ const T& At(uint32_t index) const
        {
            return m_Data[index];
        }

        __device__ __forceinline__ const T* GetData() const { return m_Data; }
        __host__ __device__ __forceinline__ uint32_t GetSize() const { return m_Size; }
    private:
        T* m_Data = nullptr;
        uint32_t m_Size = 0;
    };
}