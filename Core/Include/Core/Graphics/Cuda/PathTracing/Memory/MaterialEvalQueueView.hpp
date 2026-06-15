#pragma once

#include <cstdint>
#include <type_traits>
#include <cuda_runtime.h>
#include <Core/Graphics/Cuda/PathTracing/Memory/RayQueueView.hpp>

namespace Core::Graphics::Cuda
{
    class MaterialEvalQueueView
    {
    public:
        MaterialEvalQueueView() = default;

        MaterialEvalQueueView(
            uint32_t* size,
            uint32_t capacity,
            uint32_t* paths,
            uint32_t* depths,
            float* throughputXs,
            float* throughputYs,
            float* throughputZs,
            float* currentMediumIors,
            bool* lastScatterDeltaFlags,
            float* originXs,
            float* originYs,
            float* originZs,
            float* directionXs,
            float* directionYs,
            float* directionZs,
            float* tMins,
            float* tMaxs,
            uint32_t* triangles,
            float* us,
            float* vs)
            : m_Size(size),
              m_Capacity(capacity),
              m_Paths(paths),
              m_Depths(depths),
              m_ThroughputXs(throughputXs),
              m_ThroughputYs(throughputYs),
              m_ThroughputZs(throughputZs),
              m_CurrentMediumIors(currentMediumIors),
              m_LastScatterDeltaFlags(lastScatterDeltaFlags),
              m_OriginXs(originXs),
              m_OriginYs(originYs),
              m_OriginZs(originZs),
              m_DirectionXs(directionXs),
              m_DirectionYs(directionYs),
              m_DirectionZs(directionZs),
              m_TMins(tMins),
              m_TMaxs(tMaxs),
              m_Triangles(triangles),
              m_Us(us),
              m_Vs(vs)
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
            Path path{};
            path.index = m_Paths[index];
            path.depth = m_Depths[index];
            path.throughput = make_float3(m_ThroughputXs[index], m_ThroughputYs[index], m_ThroughputZs[index]);
            path.currentMediumIor = m_CurrentMediumIors[index];
            path.lastScatterWasDelta = m_LastScatterDeltaFlags[index];
            return path;
        }
        
        __device__ __forceinline__ PathContribution GetPathContribution(uint32_t index) const
        {
            PathContribution contribution{};
            contribution.index = m_Paths[index];
            contribution.throughput = make_float3(m_ThroughputXs[index], m_ThroughputYs[index], m_ThroughputZs[index]);
            contribution.lastScatterWasDelta = m_LastScatterDeltaFlags[index];
            return contribution;
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

        __device__ __forceinline__ float3 GetRayDirection(uint32_t index) const
        {
            return make_float3(m_DirectionXs[index], m_DirectionYs[index], m_DirectionZs[index]);
        }

        __device__ __forceinline__ float3 GetThroughput(uint32_t index) const
        {
            return make_float3(m_ThroughputXs[index], m_ThroughputYs[index], m_ThroughputZs[index]);
        }

        __device__ __forceinline__ HitData GetHitData(uint32_t index) const
        {
            HitData hitData{};
            hitData.triangle = m_Triangles[index];
            hitData.u = m_Us[index];
            hitData.v = m_Vs[index];
            return hitData;
        }

        __device__ __forceinline__ void Set(uint32_t index, const Path& path, const Ray& ray, const HitData& hitData) const
        {
            Set(index, path);
            Set(index, ray);
            Set(index, hitData);
        }

        __device__ __forceinline__ uint32_t Push(const Path& path, const Ray& ray, const HitData& hitData) const
        {
            const uint32_t index = PushIndex();
            Set(index, path, ray, hitData);
            return index;
        }
    private:
        
        __device__ __forceinline__ void Set(uint32_t index, Path path) const
        {
            m_Paths[index] = path.index;
            m_Depths[index] = path.depth;
            m_ThroughputXs[index] = path.throughput.x;
            m_ThroughputYs[index] = path.throughput.y;
            m_ThroughputZs[index] = path.throughput.z;
            m_CurrentMediumIors[index] = path.currentMediumIor;
            m_LastScatterDeltaFlags[index] = path.lastScatterWasDelta;
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

        __device__ __forceinline__ void Set(uint32_t index, const HitData& hitData) const
        {
            m_Triangles[index] = hitData.triangle;
            m_Us[index] = hitData.u;
            m_Vs[index] = hitData.v;
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
        uint32_t* __restrict__ m_Depths = nullptr;
        float* __restrict__ m_ThroughputXs = nullptr;
        float* __restrict__ m_ThroughputYs = nullptr;
        float* __restrict__ m_ThroughputZs = nullptr;
        float* __restrict__ m_CurrentMediumIors = nullptr;
        bool* __restrict__ m_LastScatterDeltaFlags = nullptr;

        float* __restrict__ m_OriginXs = nullptr;
        float* __restrict__ m_OriginYs = nullptr;
        float* __restrict__ m_OriginZs = nullptr;
        float* __restrict__ m_DirectionXs = nullptr;
        float* __restrict__ m_DirectionYs = nullptr;
        float* __restrict__ m_DirectionZs = nullptr;
        float* __restrict__ m_TMins = nullptr;
        float* __restrict__ m_TMaxs = nullptr;

        uint32_t* __restrict__ m_Triangles = nullptr;
        float* __restrict__ m_Us = nullptr;
        float* __restrict__ m_Vs = nullptr;
    };
}