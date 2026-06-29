#pragma once
#include <Core/Graphics/Cuda/Runtime/Memory/DeviceBuffer1DView.hpp>
#include <Core/Graphics/Cuda/PathTracing/Memory/AliasTableView.hpp>
#include <Core/Graphics/Cuda/Bvh/Triangle.hpp>

namespace Core::Graphics::Cuda
{
    struct LightSample
    {
        LightTriangle light;
        float selectionProbability;
    };

    class LightSamplerView
    {
    public:
        LightSamplerView(Runtime::DeviceBuffer1DView<LightTriangle> lightBuffer, AliasTableView<uint32_t> aliasTable)
            : lightBuffer(lightBuffer), aliasTable(aliasTable) {}

        __device__ __forceinline__ LightSample Sample(float u) const
        {
            const DiscreteSample<uint32_t> sample = aliasTable.Sample(u);
            return { lightBuffer.At(sample.outcomeIndex), sample.pmf };
        }
    private:
        Runtime::DeviceBuffer1DView<LightTriangle> lightBuffer;
        AliasTableView<uint32_t> aliasTable;
    };
}