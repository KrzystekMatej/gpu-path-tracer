#include <App/CameraRecorder/Layer.hpp>
#include <App/CameraRecorder/Status.hpp>
#include <App/CameraRecorder/Settings.hpp>
#include <Core/Runtime/Application.hpp>
#include <Core/Capture/Recording.hpp>
#include <Core/Capture/Resampling.hpp>

namespace App::CameraRecorder
{
	namespace
	{
		State ToCameraRecorderState(Core::Capture::MotionRecorder::State state)
		{
			switch (state)
			{
				case Core::Capture::MotionRecorder::State::Idle:
					return State::Idle;
				case Core::Capture::MotionRecorder::State::Active:
					return State::Active;
				case Core::Capture::MotionRecorder::State::Finished:
					return State::Finished;
				default:
					return State::Idle;
			}
		}
	}

	void Layer::OnAttach()
	{
		entt::registry& blackboard = Core::Runtime::Application::Blackboard();
		blackboard.ctx().emplace<Status>();
		blackboard.ctx().emplace<Settings>();

		auto& eventDispatcher = Core::Runtime::Application::EventDispatcher();
		eventDispatcher.sink<Events::Start>().connect<&Layer::OnRecordingStart>(*this);
		eventDispatcher.sink<Events::Stop>().connect<&Layer::OnRecordingStop>(*this);
		eventDispatcher.sink<Core::Ecs::SceneChangedEvent>().connect<&Layer::OnSceneChanged>(*this);
	}

	void Layer::OnDetach()
	{
		auto& eventDispatcher = Core::Runtime::Application::EventDispatcher();
		eventDispatcher.sink<Events::Start>().disconnect<&Layer::OnRecordingStart>(*this);
		eventDispatcher.sink<Events::Stop>().disconnect<&Layer::OnRecordingStop>(*this);
		eventDispatcher.sink<Core::Ecs::SceneChangedEvent>().disconnect<&Layer::OnSceneChanged>(*this);
	}

	void Layer::OnUpdate()
	{
		Core::Ecs::Scene& scene = Core::Runtime::Application::Scene();
		auto& motionRecorder = Core::Capture::GetMotionRecorder(scene, scene.GetActiveCamera());

		auto& blackboard = Core::Runtime::Application::Blackboard();
		Status& status = blackboard.ctx().get<Status>();
		status.doneFrames = motionRecorder.GetSamples().size();
		status.state = ToCameraRecorderState(motionRecorder.state);
	}

	void Layer::OnSceneChanged(const Core::Ecs::SceneChangedEvent& event)
	{
		auto& scene = Core::Runtime::Application::Scene();
		auto& settings = Core::Runtime::Application::Blackboard().ctx().get<Settings>();

		Core::Capture::InitMotionRecording(scene, scene.GetActiveCamera(), 1.0f / settings.targetFps);
	}

	void Layer::OnRecordingStart(const Events::Start& event)
	{
		Core::Ecs::Scene& scene = Core::Runtime::Application::Scene();
		auto result = Core::Capture::StartMotionRecording(scene, scene.GetActiveCamera(), 1.0f / event.settings.targetFps);
		if (!result) result.error().Log();
	}

	void Layer::OnRecordingStop(const Events::Stop& event)
	{
		Core::Ecs::Scene& scene = Core::Runtime::Application::Scene();
		auto result = Core::Capture::StopMotionRecording(scene, scene.GetActiveCamera());

		if (!result)
			return result.error().Log();

		auto& samples = result.value().get();
		std::vector<Core::Capture::MotionState> frames = Core::Capture::ResampleMotion(samples, 1.0f / event.settings.targetFps);
		Core::Runtime::Application::EventDispatcher().trigger(Events::Finish(std::move(frames)));
	}
}