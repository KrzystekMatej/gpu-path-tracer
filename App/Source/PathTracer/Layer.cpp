#include <App/PathTracer/Layer.hpp>
#include <App/PathTracer/Status.hpp>
#include <App/PathTracer/Settings.hpp>
#include <Core/Runtime/Application.hpp>

namespace App::PathTracer
{
	void Layer::OnAttach()
	{
		entt::registry& blackboard = Core::Runtime::Application::Blackboard();
		auto& status = blackboard.ctx().emplace<Status>();
		auto& settings = blackboard.ctx().emplace<Settings>();

		auto bufferResult = m_PathTracer.InitializeRenderingBuffers(settings.frameWidth, settings.frameHeight);
		if (!bufferResult)
			bufferResult.error().Log();

		m_DisplayTexture.Allocate(settings.frameWidth, settings.frameHeight);
		auto frameView = m_PathTracer.GetFrameView();
		{
			auto lock = frameView.Lock();
			const uint32_t* data = lock.GetData();
			m_DisplayTexture.Upload(data);
		}

		status.frameTexture = &m_DisplayTexture.GetTexture();

		auto& eventDispatcher = Core::Runtime::Application::EventDispatcher();

		eventDispatcher.sink<Events::Start>().connect<&Layer::OnPathTracingStart>(*this);
		eventDispatcher.sink<Events::Stop>().connect<&Layer::OnPathTracingStop>(*this);
		eventDispatcher.sink<CameraRecorder::Events::Finish>().connect<&Layer::OnCameraRecordingFinish>(*this);
		eventDispatcher.sink<Core::Ecs::SceneChangedEvent>().connect<&Layer::OnSceneChanged>(*this);
	}

	void Layer::OnSceneChanged(const Core::Ecs::SceneChangedEvent&)
	{
		m_SceneBuffersDirty = true;
		
		if (m_PathTracer.IsRendering())
		{
			spdlog::info("Scene changed while path tracer is active - path tracer will continue with the snapshot taken at the start of the simulation.");
			spdlog::info("Scene buffers will be updated on the next launch.");
		}
	}


	void Layer::OnDetach()
	{
		auto& eventDispatcher = Core::Runtime::Application::EventDispatcher();
		eventDispatcher.sink<Events::Start>().disconnect<&Layer::OnPathTracingStart>(*this);
		eventDispatcher.sink<Events::Stop>().disconnect<&Layer::OnPathTracingStop>(*this);
		eventDispatcher.sink<CameraRecorder::Events::Finish>().disconnect<&Layer::OnCameraRecordingFinish>(*this);
		eventDispatcher.sink<Core::Ecs::SceneChangedEvent>().disconnect<&Layer::OnSceneChanged>(*this);
	}

	void Layer::OnUpdate()
	{
		auto& status = Core::Runtime::Application::Blackboard().ctx().get<Status>();

		if ((status.state == State::Finished || status.state == State::Idle) && m_CameraMotionStates.size() > 0)
		{
			status.state = State::Ready;
		}
		else if ((status.state == State::Active || status.state == State::Stopping) && !m_PathTracer.IsRendering())
		{
			status.state = State::Finished;
		}

		if (status.doneFrames < m_PathTracer.GetDoneFrames())
		{
			auto frameView = m_PathTracer.GetFrameView();
			{
				auto lock = frameView.Lock();
				const uint32_t* data = lock.GetData();
				m_DisplayTexture.Upload(data);
			}
			status.doneFrames = m_PathTracer.GetDoneFrames();
		}
		
		if (m_PathTracer.PeekLastError())
		{
			auto lastError = m_PathTracer.ConsumeLastError();
			if (lastError)			
				lastError.value().Log();
		}
		status.doneSamples = m_PathTracer.GetDoneSamples();
	}

	void Layer::OnPathTracingStart(const Events::Start& event)
	{
		auto& status = Core::Runtime::Application::Blackboard().ctx().get<Status>();
		if (status.state != State::Ready)
		{
			spdlog::warn(
				"Received Path Tracing start event while in {} state"
				" - cannot start recording (Recorded frames: {})",
				ToString(status.state),
				m_CameraMotionStates.size());
			return;
		}
		
		uint32_t frameWidth = std::min(std::max(event.settings.frameWidth, Settings::Defaults::MinFrameWidth), Settings::Defaults::MaxFrameWidth);
		uint32_t frameHeight = std::min(std::max(event.settings.frameHeight, Settings::Defaults::MinFrameHeight), Settings::Defaults::MaxFrameHeight);
		if (frameWidth != event.settings.frameWidth || frameHeight != event.settings.frameHeight)
		{
			spdlog::warn(
				"Received Path Tracing start event with out-of-range frame dimensions ({}x{}) - clamping to {}x{}",
				event.settings.frameWidth, event.settings.frameHeight, frameWidth, frameHeight);
		}
		
		uint32_t samplesPerPixel = std::min(std::max(event.settings.samplesPerPixel, Settings::Defaults::MinSamplesPerPixel), Settings::Defaults::MaxSamplesPerPixel);
		if (samplesPerPixel != event.settings.samplesPerPixel)
		{
			spdlog::warn(
				"Received Path Tracing start event with out-of-range samples per pixel ({} spp) - clamping to {} spp",
				event.settings.samplesPerPixel, samplesPerPixel);
		}
		
		uint32_t pathDepthLimit = std::min(std::max(event.settings.pathDepthLimit, Settings::Defaults::MinPathDepthLimit), Settings::Defaults::MaxPathDepthLimit);
		if (pathDepthLimit != event.settings.pathDepthLimit)
		{
			spdlog::warn(
				"Received Path Tracing start event with out-of-range path depth limit ({}) - clamping to {}",
				event.settings.pathDepthLimit, pathDepthLimit);
		}

		if (frameWidth != m_PathTracer.GetFramebufferWidth() || frameHeight != m_PathTracer.GetFramebufferHeight())
		{
			m_DisplayTexture.Allocate(frameWidth, frameHeight);
		}

		if (m_SceneBuffersDirty)
		{
			auto& scene = Core::Runtime::Application::Scene();
			const auto& storage = Core::Runtime::Application::AssetManager().GetStorage();

			auto sceneBufferResult = m_PathTracer.InitializeSceneBuffers(scene.GetRegistry(), storage);
			if (!sceneBufferResult)
				return sceneBufferResult.error().Log();
			
			m_SceneBuffersDirty = false;
		}

		auto& scene = Core::Runtime::Application::Scene();
		entt::registry& registry = scene.GetRegistry();
		const auto& camera = registry.get<Core::Graphics::Ecs::Camera>(scene.GetActiveCamera());

		// TODO: add option to resume from a specific frame (should override the settings if they are different from the current state of the path tracer)
		auto result = m_PathTracer.StartSimulation(
			frameWidth, 
			frameHeight,
			camera,
			m_CameraMotionStates, 
			samplesPerPixel,
			pathDepthLimit);

		if (!result)
			return result.error().Log();

		if (event.settings.samplesPerPixel != m_PathTracer.GetSamplesPerPixel())
		{
			spdlog::warn(
				"Requested {} samples per pixel, but renderer is using {} samples per pixel (sample grid size: {})",
				event.settings.samplesPerPixel, m_PathTracer.GetSamplesPerPixel(), m_PathTracer.GetSampleGridSize());
			auto& storedSettings = Core::Runtime::Application::Blackboard().ctx().get<Settings>();
			storedSettings.samplesPerPixel = m_PathTracer.GetSamplesPerPixel();
		}

		m_StatesUpdated = false;
		status.state = State::Active;
		status.doneFrames = m_PathTracer.GetDoneFrames();
		status.totalFrames = m_PathTracer.GetTotalFrames();
		status.doneSamples = m_PathTracer.GetDoneSamples();
		status.totalSamples = m_PathTracer.GetTotalSamples();
	}

	void Layer::OnPathTracingStop(const Events::Stop&)
	{
		m_PathTracer.StopRendering();
		auto& blackboard = Core::Runtime::Application::Blackboard();

		Status& status = blackboard.ctx().get<Status>();
		if (status.state == State::Active)
		{
			status.state = State::Stopping;
		}
		else
		{
			spdlog::warn("Received Path Tracing stop event while in {} state - nothing to stop", ToString(status.state));
		}
	}

	void Layer::OnCameraRecordingFinish(const CameraRecorder::Events::Finish& event)
	{
		m_CameraMotionStates = event.frames;
		m_StatesUpdated = true;
	}
}