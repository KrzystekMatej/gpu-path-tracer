#include <Core/Graphics/Cuda/PathTracing/Renderer.hpp>
#include <Core/Graphics/Cuda/PathTracing/Kernels.hpp>

namespace Core::Graphics::Cuda::PathTracing
{
    namespace
    {
        uchar4 MakeSimulationColor(uint32_t frameIndex)
        {
            const uint8_t r = static_cast<uint8_t>((frameIndex * 37u) % 256u);
            const uint8_t g = static_cast<uint8_t>((frameIndex * 73u) % 256u);
            const uint8_t b = static_cast<uint8_t>((frameIndex * 131u) % 256u);
            return make_uchar4(r, g, b, 255);
        }
    }

    Renderer::~Renderer()
    {
        StopRendering();
    }

    std::expected<void, Core::Utils::Error> Renderer::Initialize(size_t framebufferWidth, size_t framebufferHeight)
    {
        return m_Framebuffer.Allocate(framebufferWidth, framebufferHeight, sizeof(uchar4));
    }

    Renderer::LockedFrameView Renderer::LockLatestFrame() const
    {
        std::unique_lock<std::mutex> lock(m_FrameMutex);
        return LockedFrameView(std::move(lock), m_Framebuffer);
    }

    std::optional<Core::Utils::Error> Renderer::GetLastError() const
    {
        std::scoped_lock lock(m_ErrorMutex);
        return m_LastError;
    }

    std::expected<void, Core::Utils::Error> Renderer::ResizeFramebuffer(size_t framebufferWidth, size_t framebufferHeight)
    {
        if (IsRendering())
            return std::unexpected(Core::Utils::Error("Cannot resize renderer while rendering"));

        return m_Framebuffer.Allocate(framebufferWidth, framebufferHeight, sizeof(uchar4));
    }

    std::expected<void, Core::Utils::Error> Renderer::Clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        if (IsRendering())
            return std::unexpected(Core::Utils::Error("Cannot clear renderer while rendering"));

        return RenderClear(r, g, b, a);
    }

    std::expected<void, Core::Utils::Error> Renderer::StartSimulation(uint32_t frameCount, std::chrono::milliseconds frameDelay)
    {
        if (m_IsRendering.exchange(true))
            return std::unexpected(Core::Utils::Error("Renderer is already rendering"));

        if (m_RenderThread.joinable())
            m_RenderThread.join();

        {
            std::scoped_lock lock(m_ErrorMutex);
            m_LastError.reset();
        }

        try
        {
            m_RenderThread = std::jthread([this, frameCount, frameDelay](std::stop_token stopToken)
			{
				SimulationLoop(stopToken, frameCount, frameDelay);
			});
        }
        catch (...)
        {
            m_IsRendering = false;
            return std::unexpected(Core::Utils::Error("Failed to start render thread"));
        }

        return {};
    }

    void Renderer::StopRendering()
    {
        if (m_RenderThread.joinable())
        {
            m_RenderThread.request_stop();
            m_RenderThread.join();
        }

        m_IsRendering = false;
    }

    bool Renderer::IsRendering() const
    {
        return m_IsRendering.load();
    }

    void Renderer::SimulationLoop(std::stop_token stopToken, uint32_t frameCount, std::chrono::milliseconds frameDelay)
    {
        for (uint32_t frameIndex = 0; frameIndex < frameCount; ++frameIndex)
        {
            if (stopToken.stop_requested())
                break;

            {
                std::unique_lock lock(m_StopMutex);
                m_StopCv.wait_for(lock, stopToken, frameDelay, [] { return false; });
            }

            if (stopToken.stop_requested())
                break;

            const uchar4 clearColor = MakeSimulationColor(frameIndex);
            auto renderResult = RenderClear(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

            if (!renderResult)
            {
                std::scoped_lock lock(m_ErrorMutex);
                m_LastError = renderResult.error();
                break;
            }
        }

        m_IsRendering = false;
    }

    std::expected<void, Core::Utils::Error> Renderer::RenderClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        const uchar4 clearColor = make_uchar4(r, g, b, a);

        Kernels::Clear(clearColor, m_Framebuffer.GetDeviceBuffer().GetView<uchar4>());

        std::scoped_lock lock(m_FrameMutex);
        return m_Framebuffer.CopyDeviceToHostSync();
    }
}
