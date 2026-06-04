#pragma once

#include <cstdint>
#include <expected>

#include <Core/Graphics/Cuda/PathTracing/MaterialEvalQueueView.hpp>
#include <Core/Graphics/Cuda/Runtime/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/Runtime/SharedCounter.hpp>
#include <Core/Graphics/Cuda/Runtime/Stream.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Graphics::Cuda
{
    class MaterialEvalQueue
    {
    public:
        MaterialEvalQueue() = default;
        ~MaterialEvalQueue() = default;

        MaterialEvalQueue(const MaterialEvalQueue&) = delete;
        MaterialEvalQueue& operator=(const MaterialEvalQueue&) = delete;

        MaterialEvalQueue(MaterialEvalQueue&& other) noexcept = default;
        MaterialEvalQueue& operator=(MaterialEvalQueue&& other) noexcept = default;

        std::expected<void, Core::Utils::Error> Allocate(uint32_t capacity, const Runtime::Stream& stream = Runtime::Stream::Default());
        std::expected<void, Core::Utils::Error> Free(const Runtime::Stream& stream = Runtime::Stream::Default());

        uint32_t GetCapacity() const
        {
            return m_Paths.GetSize();
        }
        
        const Runtime::SharedCounter<uint32_t>& GetCounter() const
        {
            return m_Counter;
        }

        Runtime::SharedCounter<uint32_t>& GetCounter()
        {
            return m_Counter;
        }

        MaterialEvalQueueView GetView() const
        {
            return MaterialEvalQueueView(
                m_Counter.GetDevicePointer(),
                GetCapacity(),
                reinterpret_cast<uint32_t*>(m_Paths.GetData()),
                reinterpret_cast<uint32_t*>(m_Triangles.GetData()),
                reinterpret_cast<uint32_t*>(m_Materials.GetData()),
                reinterpret_cast<float*>(m_Us.GetData()),
                reinterpret_cast<float*>(m_Vs.GetData()));
        }

    private:
        Runtime::SharedCounter<uint32_t> m_Counter;

        Runtime::DeviceBuffer1D m_Paths;
        Runtime::DeviceBuffer1D m_Triangles;
        Runtime::DeviceBuffer1D m_Materials;
        Runtime::DeviceBuffer1D m_Us;
        Runtime::DeviceBuffer1D m_Vs;
    };
}