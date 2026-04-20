#include <App/PathTracer/Layer.hpp>
#include <App/PathTracer/Status.hpp>
#include <App/PathTracer/Settings.hpp>
#include <Core/Runtime/Application.hpp>
#include <spdlog/spdlog.h>

namespace App::PathTracer
{
	void Layer::OnAttach()
	{
		entt::registry& blackboard = Core::Runtime::Application::Blackboard();
		blackboard.ctx().emplace<Status>();
		blackboard.ctx().emplace<Settings>();

		Core::Runtime::Application::EventDispatcher().sink<Ui::Events::PathTracingToggled>().connect<&Layer::OnPathTracingToggled>(*this);
	}

	void Layer::OnUpdate()
	{
	}

	void Layer::OnPathTracingToggled(const Ui::Events::PathTracingToggled& event)
	{
	}
}