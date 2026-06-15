#pragma once

#include <cstdint>
#include <type_traits>
#include <cuda_runtime.h>
#include <Core/Graphics/Cuda/PathTracing/Memory/PathData.hpp>

namespace Core::Graphics::Cuda
{
    class RegenQueueView
    {
    public:
        RegenQueueView() = default;

        RegenQueueView(
            uint32_t* size,
            uint32_t capacity,
            uint32_t* paths)
            : m_Size(size)
            , m_Capacity(capacity)
            , m_Paths(paths)
        {
        }

        __device__ __forceinline__ uint32_t GetSize() const
        {
            return *m_Size;
        }

        __device__ __forceinline__ void SetSize(uint32_t size) const
        {
            *m_Size = size;
        }

        __host__ __device__ __forceinline__ uint32_t GetCapacity() const
        {
            return m_Capacity;
        }

        __device__ __forceinline__ bool IsEmpty() const
        {
            return *m_Size == 0;
        }

        __device__ __forceinline__ bool IsFull() const
        {
            return *m_Size >= m_Capacity;
        }

        __device__ __forceinline__ void Clear() const
        {
            *m_Size = 0;
        }
        
        __device__ __forceinline__ uint32_t GetPathIndex(uint32_t index) const
        {
            return m_Paths[index];
        }

        __device__ __forceinline__ void Set(uint32_t index, uint32_t pathIndex) const
        {
            m_Paths[index] = pathIndex;
        }

        __device__ __forceinline__ uint32_t Push(uint32_t pathIndex) const
        {
            const uint32_t index = PushIndex();
            Set(index, pathIndex);
            return index;
        }
    private:
        __device__ __forceinline__ uint32_t PushIndex() const
        {
#if defined(__CUDA_ARCH__)
            return atomicAdd(m_Size, 1u);
#else
            return 0;
#endif
        }

        uint32_t* __restrict__ m_Size = nullptr;
        uint32_t m_Capacity = 0;

        uint32_t* __restrict__ m_Paths = nullptr;
    };
}