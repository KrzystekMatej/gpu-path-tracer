#include <App/CameraRecorder/Layer.hpp>
#include <App/CameraRecorder/Status.hpp>
#include <App/CameraRecorder/Settings.hpp>
#include <Core/Runtime/Application.hpp>
#include <spdlog/spdlog.h>

namespace App::CameraRecorder
{
	void Layer::OnAttach()
	{
		entt::registry& blackboard = Core::Runtime::Application::Blackboard();
		blackboard.ctx().emplace<Status>();
		blackboard.ctx().emplace<Settings>();

		Core::Runtime::Application::EventDispatcher().sink<Ui::Events::CameraRecordingToggled>().connect<&Layer::OnRecordingToggled>(*this);
	}

	void Layer::OnUpdate()
	{
	}
	void Layer::OnRecordingToggled(const Ui::Events::CameraRecordingToggled& event)
	{
	}
}