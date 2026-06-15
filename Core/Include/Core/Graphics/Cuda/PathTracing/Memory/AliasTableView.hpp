#pragma once
#include <cstdint>
#include <Core/Graphics/Cuda/Runtime/Memory/DeviceBuffer1DView.hpp>
#include <Core/Graphics/Cuda/Utils/Math.hpp>

namespace Core::Graphics::Cuda
{
    template<typename T>
    struct Bin
    {
        float q, p;
        uint32_t alias;
        T value;
    };

    template<typename T>
    struct DiscreteSample
    {
        uint32_t outcomeIndex;
        float pmf;
        float remappedU;
        T value;
    };

    template<typename T>
    class AliasTableView
    {
    public:
        AliasTableView(Runtime::DeviceBuffer1DView<Bin<T>> bins)
            : m_Bins(bins) {}

        __device__ __forceinline__ DiscreteSample<T> Sample(float u) const
        {
            const uint32_t size = m_Bins.GetSize();

            const float scaledU = u * static_cast<float>(size);
            const uint32_t binIndex = min(static_cast<uint32_t>(scaledU), size - 1u);
            const float up = fminf(scaledU - static_cast<float>(binIndex), Math::OneMinusEpsilon);

            const Bin<T> bin = m_Bins.At(binIndex);

            if (up < bin.q)
            {
                return {
                    binIndex,
                    bin.p,
                    fminf(up / bin.q, Math::OneMinusEpsilon),
                    bin.value
                };
            }

            const Bin<T> binAlias = m_Bins.At(bin.alias);

            return {
                bin.alias,
                binAlias.p,
                fminf((up - bin.q) / (1.0f - bin.q), Math::OneMinusEpsilon),
                binAlias.value
            };
        }

        __device__ __forceinline__ uint32_t GetSize() const
        {
            return m_Bins.GetSize();
        }
        
        __device__ __forceinline__ float GetPmf(uint32_t index) const
        {
            return m_Bins.At(index).p;
        }
    private:
        Runtime::DeviceBuffer1DView<Bin<T>> m_Bins;
    };
}