#pragma once

#include <cuda_runtime.h>
#include <Core/Graphics/Cuda/PathTracing/Material.hpp>
#include <Core/Graphics/Cuda/PathTracing/MaterialEvalQueueView.hpp>

namespace Core::Graphics::Cuda
{
    class MaterialEvalQueueViewsProvider
    {
    public:
        __host__ __device__ __forceinline__ MaterialEvalQueueView& At(uint32_t index)
        {
            return queues[index];
        }

        __host__ __device__ __forceinline__ const MaterialEvalQueueView& At(uint32_t index) const
        {
            return queues[index];
        }

    private:
        static constexpr size_t MaterialQueueCount = static_cast<size_t>(GlobalShadingModel::Count);
        MaterialEvalQueueView queues[MaterialQueueCount];
    };
}