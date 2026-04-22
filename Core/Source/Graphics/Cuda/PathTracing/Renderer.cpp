#include <Core/Graphics/Cuda/PathTracing/Renderer.hpp>
#include <Core/Graphics/Cuda/PathTracing/Kernels.hpp>

namespace Core::Graphics::Cuda
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

    std::expected<void, Core::Utils::Error> Renderer::Initialize(
        size_t framebufferWidth, 
        size_t framebufferHeight,
        const Ecs::Scene& scene,
        const Assets::Storage& storage)
    {
        auto result = m_Framebuffer.Allocate(framebufferWidth, framebufferHeight, sizeof(uchar4));
        if (!result)
            return std::unexpected(std::move(result).error());
        result = m_Framebuffer.GetDeviceBuffer().MemsetBytesSync(0);
		if (!result)
            return std::unexpected(std::move(result).error());
		return m_Framebuffer.CopyDeviceToHostSync();
    }

    std::optional<Core::Utils::Error> Renderer::GetLastError() const
    {
        std::scoped_lock lock(m_ErrorMutex);
        return m_LastError;
    }

    std::expected<void, Core::Utils::Error> Renderer::Clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        if (IsRendering())
            return std::unexpected(Core::Utils::Error("Cannot clear renderer while rendering"));

        return RenderClear(r, g, b, a);
    }

    std::expected<void, Core::Utils::Error> Renderer::StartSimulation(
		uint32_t frameWidth,
		uint32_t frameHeight,
		std::vector<Capture::MotionState> cameraMotionStates,
        uint32_t startFrame,
		uint32_t samplesPerPixel,
		uint32_t pathDepth,
        std::chrono::milliseconds frameDelay)
    {
        if (m_IsRendering.exchange(true))
            return std::unexpected(Core::Utils::Error("Renderer is already rendering"));

        if (m_RenderThread.joinable())
            m_RenderThread.join();

        {
            std::scoped_lock lock(m_ErrorMutex);
            m_LastError.reset();
        }

        if (startFrame >= cameraMotionStates.size())
		{
            m_IsRendering.store(false, std::memory_order_relaxed);
            return std::unexpected(Core::Utils::Error("Start frame is out of range - nothing to render"));
        }

        if (frameWidth != m_Framebuffer.GetWidth() || frameHeight != m_Framebuffer.GetHeight())
        {
            auto result = m_Framebuffer.Allocate(frameWidth, frameHeight, sizeof(uchar4));
            if (!result)
            {
				m_IsRendering.store(false, std::memory_order_relaxed);
                return std::unexpected(std::move(result).error());
            }
		}

		m_CameraMotionStates = std::move(cameraMotionStates);
		m_SampleGridSize = static_cast<uint32_t>(std::sqrt(samplesPerPixel));
		m_SamplesPerPixel = m_SampleGridSize * m_SampleGridSize;
		m_PathDepth = pathDepth;

        m_DoneFrames.store(startFrame, std::memory_order_relaxed);
		m_TotalFrames = static_cast<uint32_t>(m_CameraMotionStates.size());
		m_DoneSamples.store(0, std::memory_order_relaxed);
		m_TotalSamples = m_SamplesPerPixel * frameWidth * frameHeight;

        try
        {
            m_RenderThread = std::jthread([this, startFrame, frameDelay](std::stop_token stopToken)
			{
				SimulationLoop(stopToken, startFrame, frameDelay);
			});
        }
        catch (...)
        {
			m_IsRendering.store(false, std::memory_order_relaxed);
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

    void Renderer::SimulationLoop(std::stop_token stopToken, uint32_t startFrame, std::chrono::milliseconds frameDelay)
    {
        for (uint32_t frameIndex = startFrame; frameIndex < m_TotalFrames; ++frameIndex)
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

			m_DoneFrames.fetch_add(1, std::memory_order_seq_cst);
        }

        m_IsRendering.store(false, std::memory_order_relaxed);
    }

    std::expected<void, Core::Utils::Error> Renderer::RenderClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        const uchar4 clearColor = make_uchar4(r, g, b, a);

        Kernels::Clear(clearColor, m_Framebuffer.GetDeviceBuffer().GetView<uchar4>());

        std::scoped_lock lock(m_FrameMutex);
        return m_Framebuffer.CopyDeviceToHostSync();
    }
}
