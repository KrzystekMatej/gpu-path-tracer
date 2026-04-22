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

#include <Core/Graphics/Cuda/Memory/SharedBuffer2D.hpp>
#include <Core/Assets/Storage.hpp>
#include <Core/Ecs/Scene.hpp>
#include <Core/Capture/Sample.hpp>
#include <Core/Utils/Error.hpp>

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
                const Memory::SharedBuffer2D& buffer) :
                m_Lock(std::move(lock)),
                m_Buffer(buffer)
            {
            }

            const uint32_t* GetData() const
            {
                return m_Buffer.GetHost<uint32_t>();
            }

            size_t GetWidth() const
            {
                return m_Buffer.GetWidth();
            }

            size_t GetHeight() const
            {
                return m_Buffer.GetHeight();
            }

            size_t GetElementSize() const
            {
                return m_Buffer.GetElementSize();
            }

        private:
            std::unique_lock<std::mutex> m_Lock;
            const Memory::SharedBuffer2D& m_Buffer;
        };

        class FrameView
        {
        public:
			FrameView() = default;
            explicit FrameView(const Renderer& renderer) :
                m_Renderer(&renderer)
            {
            }

            size_t GetWidth() const
            {
                return m_Renderer->m_Framebuffer.GetWidth();
            }

            size_t GetHeight() const
            {
                return m_Renderer->m_Framebuffer.GetHeight();
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

        std::expected<void, Core::Utils::Error> Initialize(
            size_t framebufferWidth, 
            size_t framebufferHeight,
			const Ecs::Scene& scene,
			const Assets::Storage& storage
        );

		size_t GetFramebufferWidth() const { return m_Framebuffer.GetWidth(); }
		size_t GetFramebufferHeight() const { return m_Framebuffer.GetHeight(); }
        std::optional<Core::Utils::Error> GetLastError() const;
        FrameView GetFrameView() const { return FrameView(*this); }

        std::expected<void, Core::Utils::Error> Clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        std::expected<void, Core::Utils::Error> StartSimulation(
            uint32_t frameWidth,
            uint32_t frameHeight,
            std::vector<Capture::MotionState> cameraMotionStates,
            uint32_t startFrame,
			uint32_t samplesPerPixel,
			uint32_t pathDepth,
            std::chrono::milliseconds frameDelay);
        void StopRendering();

		bool IsRendering() const { return m_IsRendering.load(); }
		uint32_t GetDoneFrames() const { return m_DoneFrames.load(); }
		uint32_t GetTotalFrames() const { return m_TotalFrames; }
		uint32_t GetDoneSamples() const { return m_DoneSamples.load(); }
		uint32_t GetTotalSamples() const { return m_TotalSamples; }
    private:
        void SimulationLoop(std::stop_token stopToken, uint32_t startFrame, std::chrono::milliseconds frameDelay);
        std::expected<void, Core::Utils::Error> RenderClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        Memory::SharedBuffer2D m_Framebuffer;

        mutable std::mutex m_FrameMutex;
        mutable std::mutex m_ErrorMutex;

        std::jthread m_RenderThread;
        std::atomic<bool> m_IsRendering = false;

        std::condition_variable_any m_StopCv;
        std::mutex m_StopMutex;

        std::optional<Core::Utils::Error> m_LastError;

		std::atomic<uint32_t> m_DoneFrames = 0;
		uint32_t m_TotalFrames = 0;
		std::atomic<uint32_t> m_DoneSamples = 0;
		uint32_t m_TotalSamples = 0;

		std::vector<Capture::MotionState> m_CameraMotionStates;

        uint32_t m_SampleGridSize = 10;
		uint32_t m_SamplesPerPixel = 100;
        uint32_t m_PathDepth = 10;
    };
}
