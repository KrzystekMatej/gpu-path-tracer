#pragma once
#include <Core/Graphics/Cuda/PathTracing/Memory/PathData.hpp>

namespace Core::Graphics::Cuda
{
    class ShadowRayQueueView
    {
    public:
        ShadowRayQueueView() = default;

        ShadowRayQueueView(
            uint32_t* size,
            uint32_t capacity,
            uint32_t* paths,
            float* radianceXs,
            float* radianceYs,
            float* radianceZs,
            float* originXs,
            float* originYs,
            float* originZs,
            float* directionXs,
            float* directionYs,
            float* directionZs,
            float* tMins,
            float* tMaxs)
            : m_Size(size),
              m_Capacity(capacity),
              m_Paths(paths),
              m_RadianceXs(radianceXs),
              m_RadianceYs(radianceYs),
              m_RadianceZs(radianceZs),
              m_OriginXs(originXs),
              m_OriginYs(originYs),
              m_OriginZs(originZs),
              m_DirectionXs(directionXs),
              m_DirectionYs(directionYs),
              m_DirectionZs(directionZs),
              m_TMins(tMins),
              m_TMaxs(tMaxs)
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

        __device__ __forceinline__ float3 GetRadiance(uint32_t index) const
        {
            return make_float3(m_RadianceXs[index], m_RadianceYs[index], m_RadianceZs[index]);
        }

        __device__ __forceinline__ Ray GetRay(uint32_t index) const
        {
            Ray ray{};
            ray.origin = make_float3(m_OriginXs[index], m_OriginYs[index], m_OriginZs[index]);
            ray.direction = make_float3(m_DirectionXs[index], m_DirectionYs[index], m_DirectionZs[index]);
            ray.tMin = m_TMins[index];
            ray.tMax = m_TMaxs[index];
            return ray;
        }

        __device__ __forceinline__ void Set(uint32_t index, uint32_t pathIndex) const
        {
            m_Paths[index] = pathIndex;
        }

        __device__ __forceinline__ void Set(uint32_t index, float3 radiance) const
        {
            m_RadianceXs[index] = radiance.x;
            m_RadianceYs[index] = radiance.y;
            m_RadianceZs[index] = radiance.z;
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
        }

        __device__ __forceinline__ uint32_t Push(uint32_t pathIndex, float3 radiance, const Ray& ray) const
        {
            const uint32_t index = PushIndex();
            Set(index, pathIndex);
            Set(index, radiance);
            Set(index, ray);
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
        float* __restrict__ m_RadianceXs = nullptr;
        float* __restrict__ m_RadianceYs = nullptr;
        float* __restrict__ m_RadianceZs = nullptr;
        float* __restrict__ m_OriginXs = nullptr;
        float* __restrict__ m_OriginYs = nullptr;
        float* __restrict__ m_OriginZs = nullptr;
        float* __restrict__ m_DirectionXs = nullptr;
        float* __restrict__ m_DirectionYs = nullptr;
        float* __restrict__ m_DirectionZs = nullptr;
        float* __restrict__ m_TMins = nullptr;
        float* __restrict__ m_TMaxs = nullptr;
    };
}