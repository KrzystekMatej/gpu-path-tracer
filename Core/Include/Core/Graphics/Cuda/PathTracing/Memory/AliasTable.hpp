#pragma once
#include <Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp>
#include <Core/Graphics/Cuda/PathTracing/Memory/AliasTableView.hpp>
#include <Core/Graphics/Cuda/Runtime/Memory/DeviceBuffer1D.hpp>
#include <numeric>

namespace Core::Graphics::Cuda
{
    template<typename T>
    class AliasTable
    {
    public:
        AliasTable() = default;
        ~AliasTable() = default;

        AliasTable(const AliasTable&) = delete;
        AliasTable& operator=(const AliasTable&) = delete;

        AliasTable(AliasTable&& other) noexcept = default;
        AliasTable& operator=(AliasTable&& other) noexcept = default;

        std::expected<void, Core::Utils::Error> BuildSync(const std::vector<float>& weights, const std::vector<T>& values, const Runtime::Stream& stream = Runtime::Stream::Default())
        {
            float sum = static_cast<float>(std::accumulate(weights.begin(), weights.end(), 0.0));
            assert(sum > 0.0f);
            
            std::vector<Bin<T>> bins(weights.size());
            for (uint32_t i = 0; i < weights.size(); i++)
            {
                bins[i] = { .q = 0.0f, .p = weights[i] / sum, .alias = 0u, .value = values[i] };
            }

            struct Outcome 
            {
                float pHat;
                uint32_t index;
            };

            std::vector<Outcome> under, over;
            for (uint32_t i = 0; i < bins.size(); i++) 
            {
                float pHat = bins[i].p * bins.size();
                if (pHat < 1)
                    under.push_back(Outcome{pHat, i});
                else
                    over.push_back(Outcome{pHat, i});
            }

            while (!under.empty() && !over.empty()) 
            {
                Outcome un = under.back(), ov = over.back();
                under.pop_back();
                over.pop_back();

                bins[un.index].q = un.pHat;
                bins[un.index].alias = ov.index;

                float pExcess = un.pHat + ov.pHat - 1;
                if (pExcess < 1)
                    under.push_back(Outcome{pExcess, ov.index});
                else
                    over.push_back(Outcome{pExcess, ov.index});
            }

            while (!over.empty()) 
            {
                Outcome ov = over.back();
                over.pop_back();
                bins[ov.index].q = 1;
                bins[ov.index].alias = PathTracerDefaults::InvalidIndex;
            }
            while (!under.empty()) 
            {
                Outcome un = under.back();
                under.pop_back();
                bins[un.index].q = 1;
                bins[un.index].alias = PathTracerDefaults::InvalidIndex;
            }

            CORE_TRY_DISCARD_CONTEXT(m_Bins.Allocate(static_cast<uint32_t>(bins.size()), static_cast<uint32_t>(sizeof(Bin<T>)), stream), "Failed to allocate alias table buffer");
            CORE_TRY_DISCARD_CONTEXT(m_Bins.Upload(bins.data(), static_cast<uint32_t>(bins.size()), stream), "Failed to upload alias table data");
            CORE_TRY_DISCARD_CONTEXT(stream.Synchronize(), "Failed to synchronize after building alias table");
            return {};
        }

        std::expected<void, Core::Utils::Error> Free(const Runtime::Stream& stream = Runtime::Stream::Default())
        {
            return m_Bins.Free(stream);
        }

        uint32_t GetSize() const
        {
            return m_Bins.GetSize();
        }

        AliasTableView<T> GetView() const
        {
            return AliasTableView<T>(m_Bins.GetView<Bin<T>>());
        }

    private:
        Runtime::DeviceBuffer1D m_Bins;
    };
}
