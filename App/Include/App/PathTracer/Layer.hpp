#pragma once
#include <Core/Runtime/Layer/Base.hpp>
#include <App/Ui/Events.hpp>

namespace App::PathTracer
{
	class Layer : public Core::Runtime::Layer::Base
	{
	public:
		void OnAttach() override;
		void OnUpdate() override;
	private:
		void OnPathTracingToggled(const Ui::Events::PathTracingToggled& event);
	};
}