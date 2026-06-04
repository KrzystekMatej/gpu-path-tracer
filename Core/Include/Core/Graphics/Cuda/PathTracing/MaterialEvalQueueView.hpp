#pragma once

#include <cstdint>
#include <cuda_runtime.h>
#include <Core/Graphics/Cuda/PathTracing/PathData.hpp>

namespace Core::Graphics::Cuda
{
    class MaterialEvalQueueView
    {
    public:
        class HitDataRef
        {
        public:
            __device__ __forceinline__ HitDataRef(
                uint32_t index,
                uint32_t* paths,
                uint32_t* triangles,
                uint32_t* materials,
                float* us,
                float* vs)
                : m_Index(index)
                , m_Paths(paths)
                , m_Triangles(triangles)
                , m_Materials(materials)
                , m_Us(us)
                , m_Vs(vs)
            {
            }

            __device__ __forceinline__ operator HitData() const
            {
                HitData hitData;
                hitData.path = m_Paths[m_Index];
                hitData.triangle = m_Triangles[m_Index];
                hitData.material = m_Materials[m_Index];
                hitData.u = m_Us[m_Index];
                hitData.v = m_Vs[m_Index];
                return hitData;
            }

            __device__ __forceinline__ HitDataRef& operator=(const HitData& hitData)
            {
                m_Paths[m_Index] = hitData.path;
                m_Triangles[m_Index] = hitData.triangle;
                m_Materials[m_Index] = hitData.material;
                m_Us[m_Index] = hitData.u;
                m_Vs[m_Index] = hitData.v;
                return *this;
            }

        private:
            uint32_t m_Index = 0;
            uint32_t* m_Paths = nullptr;
            uint32_t* m_Triangles = nullptr;
            uint32_t* m_Materials = nullptr;
            float* m_Us = nullptr;
            float* m_Vs = nullptr;
        };

        MaterialEvalQueueView() = default;

        MaterialEvalQueueView(
            uint32_t* size,
            uint32_t capacity,
            uint32_t* paths,
            uint32_t* triangles,
            uint32_t* materials,
            float* us,
            float* vs)
            : m_Size(size)
            , m_Capacity(capacity)
            , m_Paths(paths)
            , m_Triangles(triangles)
            , m_Materials(materials)
            , m_Us(us)
            , m_Vs(vs)
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

        __device__ __forceinline__ HitDataRef At(uint32_t index)
        {
            return HitDataRef(index, m_Paths, m_Triangles, m_Materials, m_Us, m_Vs);
        }

        __device__ __forceinline__ HitData At(uint32_t index) const
        {
            HitData hitData;
            hitData.path = m_Paths[index];
            hitData.triangle = m_Triangles[index];
            hitData.material = m_Materials[index];
            hitData.u = m_Us[index];
            hitData.v = m_Vs[index];
            return hitData;
        }

        __device__ __forceinline__ void Set(uint32_t index, const HitData& hitData) const
        {
            m_Paths[index] = hitData.path;
            m_Triangles[index] = hitData.triangle;
            m_Materials[index] = hitData.material;
            m_Us[index] = hitData.u;
            m_Vs[index] = hitData.v;
        }

        __device__ __forceinline__ bool PushChecked(const HitData& hitData) const
        {
#if defined(__CUDA_ARCH__)
            const uint32_t index = atomicAdd(m_Size, 1u);
#else
            const uint32_t index = 0;
#endif

            if (index >= m_Capacity)
            {
#if defined(__CUDA_ARCH__)
                atomicSub(m_Size, 1u);
#endif
                return false;
            }

            Set(index, hitData);
            return true;
        }

        __device__ __forceinline__ uint32_t Push(const HitData& hitData) const
        {
#if defined(__CUDA_ARCH__)
            const uint32_t index = atomicAdd(m_Size, 1u);
#else
            const uint32_t index = 0;
#endif

            Set(index, hitData);
            return index;
        }

        __device__ __forceinline__ uint32_t* GetPaths() const { return m_Paths; }
        __device__ __forceinline__ uint32_t* GetTriangles() const { return m_Triangles; }
        __device__ __forceinline__ uint32_t* GetMaterials() const { return m_Materials; }
        __device__ __forceinline__ float* GetUs() const { return m_Us; }
        __device__ __forceinline__ float* GetVs() const { return m_Vs; }

    private:
        uint32_t* m_Size = nullptr;
        uint32_t m_Capacity = 0;

        uint32_t* m_Paths = nullptr;
        uint32_t* m_Triangles = nullptr;
        uint32_t* m_Materials = nullptr;
        float* m_Us = nullptr;
        float* m_Vs = nullptr;
    };
}