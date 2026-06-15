#pragma once
#include <Core/Graphics/Cuda/PathTracing/Memory/LightSamplerView.hpp>
#include <Core/Graphics/Cuda/Runtime/Memory/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/PathTracing/Memory/AliasTable.hpp>

namespace Core::Graphics::Cuda
{
    class LightSampler
    {
    public:
        LightSampler() = default;
        ~LightSampler() = default;

        LightSampler(const LightSampler&) = delete;
        LightSampler& operator=(const LightSampler&) = delete;

        LightSampler(LightSampler&& other) noexcept = default;
        LightSampler& operator=(LightSampler&& other) noexcept = default;

        std::expected<void, Core::Utils::Error> BuildSync(const std::vector<Triangle>& triangles, const std::vector<Material>& materials, const Runtime::Stream& stream = Runtime::Stream::Default());
        std::expected<void, Core::Utils::Error> Free(const Runtime::Stream& stream = Runtime::Stream::Default());

        LightSamplerView GetView() const
        {
            return LightSamplerView(m_LightBuffer.GetView<LightTriangle>(), m_AliasTable.GetView());
        }
    private:
        Runtime::DeviceBuffer1D m_LightBuffer;
        AliasTable<uint32_t> m_AliasTable;
    };
}