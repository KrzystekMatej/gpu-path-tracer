#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <mutex>
#include <optional>
#include <stop_token>
#include <thread>
#include <utility>

#include <Core/Graphics/Cuda/Runtime/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/Runtime/DeviceQueue.hpp>
#include <Core/Graphics/Cuda/Runtime/SharedBuffer1D.hpp>
#include <Core/Graphics/Cuda/Runtime/SharedCounter.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathPool.hpp>
#include <Core/Assets/Storage.hpp>
#include <Core/Ecs/Scene.hpp>
#include <Core/Capture/Sample.hpp>
#include <Core/Utils/Error.hpp>
#include <Core/Graphics/Cuda/Bvh/DeviceBvh.hpp>
#include <Core/Graphics/Ecs/Camera.hpp>
#include <Core/Graphics/Cuda/PathTracing/DeviceCamera.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp>
#include <Core/Graphics/Cuda/PathTracing/Material.hpp>
#include <Core/Graphics/Cuda/PathTracing/RayQueue.hpp>
#include <Core/Graphics/Cuda/PathTracing/MaterialEvalQueue.hpp>
#include <Core/Graphics/Cuda/PathTracing/RegenQueue.hpp>
#include <Core/Graphics/Cuda/Runtime/Stream.hpp>

namespace Core::Graphics::Cuda
{
    class Renderer
    {
    public:
        class LockedFrameView
        {
        public:
            LockedFrameView(
                std::unique_lock<std::mutex>&& lock,
                const Runtime::SharedBuffer1D& buffer) :
                m_Lock(std::move(lock)),
                m_Buffer(buffer)
            {
            }

            const uint32_t* GetData() const
            {
                return m_Buffer.GetHost<uint32_t>();
            }

            size_t GetSize() const
            {
                return m_Buffer.GetSize();
            }

            size_t GetElementSize() const
            {
                return m_Buffer.GetElementSize();
            }

        private:
            std::unique_lock<std::mutex> m_Lock;
            const Runtime::SharedBuffer1D& m_Buffer;
        };

        class FrameView
        {
        public:
			FrameView() = default;
            explicit FrameView(const Renderer& renderer) :
                m_Renderer(&renderer)
            {
            }

            size_t GetSize() const
            {
                return m_Renderer->m_Framebuffer.GetSize();
            }
            
            size_t GetElementSize() const
            {
                return m_Renderer->m_Framebuffer.GetElementSize();
            }

            LockedFrameView Lock() const
            {
                std::unique_lock<std::mutex> lock(m_Renderer->m_FrameMutex);
                return LockedFrameView(std::move(lock), m_Renderer->m_Framebuffer);
            }

		private:
			const Renderer* m_Renderer = nullptr;
		};


        Renderer() = default;
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        std::expected<void, Core::Utils::Error> InitializeRenderingBuffers(uint32_t width, uint32_t height);
		std::expected<void, Core::Utils::Error> InitializeSceneBuffers(const entt::registry& sceneRegistry, const Assets::Storage& storage);

		uint32_t GetFramebufferWidth() const { return m_Width; }
		uint32_t GetFramebufferHeight() const { return m_Height; }
        std::optional<Core::Utils::Error> PeekLastError() const;
        std::optional<Core::Utils::Error> ConsumeLastError();

        FrameView GetFrameView() const { return FrameView(*this); }

        std::expected<void, Core::Utils::Error> Clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        std::expected<void, Core::Utils::Error> StartSimulation(
            uint32_t width,
            uint32_t height,
            const Graphics::Ecs::Camera& camera,
            std::vector<Capture::MotionState> cameraMotionStates,
			uint32_t samplesPerPixel,
			uint32_t pathDepth);
        std::expected<void, Core::Utils::Error> ResumeSimulation(uint32_t startFrame);
        std::expected<void, Core::Utils::Error> StopRendering();

		bool IsRendering() const { return m_IsRendering.load(); }
		uint32_t GetDoneFrames() const { return m_DoneFrames.load(); }
		uint32_t GetTotalFrames() const { return m_TotalFrames; }
		uint64_t GetDoneSamples() const { return m_DoneSamples.load(); }
		uint64_t GetTotalSamples() const { return m_TotalSamples; }
        uint32_t GetSampleGridSize() const { return m_SampleGridSize; }
        uint32_t GetSamplesPerPixel() const { return m_SamplesPerPixel; }
        uint32_t GetPathDepth() const { return m_PathDepthLimit; }

        static constexpr size_t PathPoolSize = 2 << 19;
    private:
        void SimulationLoop(std::stop_token stopToken, uint32_t startFrame);
        std::expected<void, Core::Utils::Error> RenderFrame(std::stop_token stopToken, DeviceCamera camera, uint32_t generateCount);
        std::expected<void, Core::Utils::Error> RenderClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        Runtime::SharedBuffer1D m_Framebuffer;
		Runtime::DeviceBuffer1D m_AccumulationBuffer;
        uint32_t m_Width = PathTracerDefaults::FrameWidth;
        uint32_t m_Height = PathTracerDefaults::FrameHeight;

        mutable std::mutex m_FrameMutex;
        mutable std::mutex m_ErrorMutex;

        std::jthread m_RenderThread;
        std::atomic<bool> m_IsRendering = false;

        std::condition_variable_any m_StopCv;
        std::mutex m_StopMutex;

        std::optional<Core::Utils::Error> m_LastError;

		std::atomic<uint32_t> m_DoneFrames = 0;
		uint32_t m_TotalFrames = 0;
		std::atomic<uint64_t> m_DoneSamples = 0;
		uint64_t m_TotalSamples = 0;

        Graphics::Ecs::Camera m_Camera;
		std::vector<Capture::MotionState> m_CameraMotionStates;

        uint32_t m_SampleGridSize = PathTracerDefaults::SampleGridSize;
		uint32_t m_SamplesPerPixel = PathTracerDefaults::SamplesPerPixel;
        uint32_t m_PathDepthLimit = PathTracerDefaults::PathDepthLimit;
        
        Runtime::Stream m_RenderStream;

        Runtime::DeviceBuffer1D m_MaterialBuffer;
        std::array<MaterialEvalQueue, static_cast<size_t>(GlobalShadingModel::Count)> m_MaterialEvalQueues;
        std::array<uint32_t, static_cast<size_t>(GlobalShadingModel::Count)> m_MaterialCounts;
		DeviceBvh m_Bvh;

        PathPool m_PathPool;
        std::array<RayQueue, 2> m_RayQueues;
        RegenQueue m_RegenQueue;
    };
}
