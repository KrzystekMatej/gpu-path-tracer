#pragma once
#include <cstddef>
#include <cuda_runtime.h>

namespace Core::Graphics::Cuda::Runtime
{
    template<typename T>
    class DeviceBuffer2DView
    {
    public:
        DeviceBuffer2DView(T* data, uint32_t width, uint32_t height, uint32_t pitchBytes)
            : m_Data(data), m_Width(width), m_Height(height), m_PitchBytes(pitchBytes) {}

        
        __device__ __forceinline__ T& At(uint32_t row, uint32_t col)
        {
            return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(m_Data) + row * m_PitchBytes + col * sizeof(T));
        }

        __device__ __forceinline__ const T& At(uint32_t row, uint32_t col) const
        {
            return *reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(m_Data) + row * m_PitchBytes + col * sizeof(T));
        }
        
        __device__ __forceinline__ const T* GetData() const { return m_Data; }
        __host__ __device__ __forceinline__ uint32_t GetWidth() const { return m_Width; }
        __host__ __device__ __forceinline__ uint32_t GetHeight() const { return m_Height; }
        __host__ __device__ __forceinline__ uint32_t GetPitchBytes() const { return m_PitchBytes; }
    private:
        T* __restrict__ m_Data = nullptr;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_PitchBytes = 0;
    };
}