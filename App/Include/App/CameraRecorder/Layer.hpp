#pragma once
#include <Core/Runtime/Layer/Base.hpp>
#include <App/Ui/Events.hpp>

namespace App::CameraRecorder
{
	class Layer : public Core::Runtime::Layer::Base
	{
	public:
		void OnAttach() override;
		void OnUpdate() override;
	private:
		void OnRecordingToggled(const Ui::Events::CameraRecordingToggled& event);
	};
}