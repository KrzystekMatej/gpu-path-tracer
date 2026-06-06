#pragma once

#include <cstdint>
#include <type_traits>
#include <cuda_runtime.h>
#include <Core/Graphics/Cuda/PathTracing/PathData.hpp>

namespace Core::Graphics::Cuda
{
    class RayQueueView
    {
    public:
        RayQueueView() = default;

        RayQueueView(
            uint32_t* size,
            uint32_t capacity,
            uint32_t* paths,
            float* originXs,
            float* originYs,
            float* originZs,
            float* directionXs,
            float* directionYs,
            float* directionZs,
            float* tMins,
            float* tMaxs,
            float* iors)
            : m_Size(size)
            , m_Capacity(capacity)
            , m_Paths(paths)
            , m_OriginXs(originXs)
            , m_OriginYs(originYs)
            , m_OriginZs(originZs)
            , m_DirectionXs(directionXs)
            , m_DirectionYs(directionYs)
            , m_DirectionZs(directionZs)
            , m_TMins(tMins)
            , m_TMaxs(tMaxs)
            , m_Iors(iors)
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
        
        __device__ __forceinline__ Path GetPath(uint32_t index) const
        {
            return Path{ m_Paths[index] };
        }
        
        __device__ __forceinline__ Ray GetRay(uint32_t index) const
        {
            Ray ray{};
            ray.origin = make_float3(m_OriginXs[index], m_OriginYs[index], m_OriginZs[index]);
            ray.direction = make_float3(m_DirectionXs[index], m_DirectionYs[index], m_DirectionZs[index]);
            ray.tMin = m_TMins[index];
            ray.tMax = m_TMaxs[index];
            ray.ior = m_Iors[index];
            return ray;
        }   

        __device__ __forceinline__ void Set(uint32_t index, const Path& path, const Ray& ray) const
        {
            Set(index, path);
            Set(index, ray);
        }

        __device__ __forceinline__ uint32_t Push(const Path& path, const Ray& ray) const
        {
            const uint32_t index = PushIndex();
            Set(index, path, ray);
            return index;
        }

    private:
        
        __device__ __forceinline__ void Set(uint32_t index, Path path) const
        {
            m_Paths[index] = path.index;
        }

        __device__ __forceinline__ void Set(uint32_t index, const Ray& ray) const
        {
            m_OriginXs[index] = ray.origin.x;
            m_OriginYs[index] = ray.origin.y;
            m_OriginZs[index] = ray.origin.z;
            m_DirectionXs[index] = ray.direction.x;
            m_DirectionYs[index] = ray.direction.y;
            m_DirectionZs[index] = ray.direction.z;
            m_TMins[index] = ray.tMin;
            m_TMaxs[index] = ray.tMax;
            m_Iors[index] = ray.ior;
        }

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
        float* __restrict__ m_OriginXs = nullptr;
        float* __restrict__ m_OriginYs = nullptr;
        float* __restrict__ m_OriginZs = nullptr;
        float* __restrict__ m_DirectionXs = nullptr;
        float* __restrict__ m_DirectionYs = nullptr;
        float* __restrict__ m_DirectionZs = nullptr;
        float* __restrict__ m_TMins = nullptr;
        float* __restrict__ m_TMaxs = nullptr;
        float* __restrict__ m_Iors = nullptr;
    };
}