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
#include <Core/Utils/Error.hpp>

namespace Core::Graphics::Cuda::PathTracing
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

        Renderer() = default;
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        std::expected<void, Core::Utils::Error> Initialize(size_t framebufferWidth, size_t framebufferHeight);

		size_t GetFramebufferWidth() const { return m_Framebuffer.GetWidth(); }
		size_t GetFramebufferHeight() const { return m_Framebuffer.GetHeight(); }
        std::optional<Core::Utils::Error> GetLastError() const;
        LockedFrameView LockLatestFrame() const;

        std::expected<void, Core::Utils::Error> ResizeFramebuffer(size_t width, size_t height);
        std::expected<void, Core::Utils::Error> Clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        std::expected<void, Core::Utils::Error> StartSimulation(uint32_t frameCount, std::chrono::milliseconds frameDelay);
        void StopRendering();
        bool IsRendering() const;
    private:
        void SimulationLoop(std::stop_token stopToken, uint32_t frameCount, std::chrono::milliseconds frameDelay);
        std::expected<void, Core::Utils::Error> RenderClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        Memory::SharedBuffer2D m_Framebuffer;

        mutable std::mutex m_FrameMutex;
        mutable std::mutex m_ErrorMutex;

        std::jthread m_RenderThread;
        std::atomic<bool> m_IsRendering = false;

        std::condition_variable_any m_StopCv;
        std::mutex m_StopMutex;

        std::optional<Core::Utils::Error> m_LastError;
    };
}
