#pragma once
#include <cuda_runtime.h>
#include <Core/Graphics/Cuda/PathTracing/Material.hpp>
#include <Core/Graphics/Cuda/Memory/DeviceQueueView.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathData.hpp>

namespace Core::Graphics::Cuda
{
    class MaterialQueueViews
    {
    public:
        __host__ __device__ __forceinline__ Memory::DeviceQueueView<HitData>& At(uint32_t index)
        {
            return queues[index];
        }

        __host__ __device__ __forceinline__ const Memory::DeviceQueueView<HitData>& At(uint32_t index) const
        {
            return queues[index];
        }
    private:
        static constexpr size_t MaterialQueueCount = static_cast<size_t>(GlobalShadingModel::Count);
        Memory::DeviceQueueView<HitData> queues[MaterialQueueCount];
    };
}
