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

		m_PathTracer.Initialize(settings.frameWidth, settings.frameHeight);
		status.currentFrame = m_PathTracer.GetFrameView();

		auto& eventDispatcher = Core::Runtime::Application::EventDispatcher();

		eventDispatcher.sink<Events::Start>().connect<&Layer::OnPathTracingStart>(*this);
		eventDispatcher.sink<Events::Stop>().connect<&Layer::OnPathTracingStop>(*this);
		eventDispatcher.sink<CameraRecorder::Events::Finish>().connect<&Layer::OnCameraRecordingFinish>(*this);

	}

	void Layer::OnDetach()
	{
		auto& eventDispatcher = Core::Runtime::Application::EventDispatcher();
		eventDispatcher.sink<Events::Start>().disconnect<&Layer::OnPathTracingStart>(*this);
		eventDispatcher.sink<Events::Stop>().disconnect<&Layer::OnPathTracingStop>(*this);
		eventDispatcher.sink<CameraRecorder::Events::Finish>().disconnect<&Layer::OnCameraRecordingFinish>(*this);
	}

	void Layer::OnUpdate()
	{
		auto& status = Core::Runtime::Application::Blackboard().ctx().get<Status>();

		if ((status.state == State::Finished || status.state == State::Idle) && m_RecordedFrames.size() > 0)
		{
			status.state = State::Ready;
		}
		else if (status.state == State::Stopping && !m_PathTracer.IsRendering())
		{
			status.state = State::Finished;
		}
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
				m_RecordedFrames.size());
			return;
		}

		if (event.settings.frameWidth != m_PathTracer.GetFramebufferWidth() || event.settings.frameHeight != m_PathTracer.GetFramebufferHeight())
			m_PathTracer.ResizeFramebuffer(event.settings.frameWidth, event.settings.frameHeight);

		auto duration = std::chrono::milliseconds(1000);
		m_PathTracer.StartSimulation(m_RecordedFrames.size(), duration);
		status.state = State::Active;
	}

	void Layer::OnPathTracingStop(const Events::Stop& event)
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
		m_RecordedFrames = event.frames;
	}
}