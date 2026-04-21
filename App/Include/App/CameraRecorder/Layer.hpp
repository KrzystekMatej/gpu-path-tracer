#pragma once
#include <Core/Runtime/Layer/Base.hpp>
#include <App/CameraRecorder/Events.hpp>

namespace App::CameraRecorder
{
	class Layer : public Core::Runtime::Layer::Base
	{
	public:
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
	private:
		void OnSceneChanged(const Core::Ecs::SceneChangedEvent& event);
		void OnRecordingStart(const Events::Start& event);
		void OnRecordingStop(const Events::Stop& event);
	};
}