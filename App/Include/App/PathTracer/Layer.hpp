#pragma once
#include <Core/Runtime/Layer/Base.hpp>
#include <App/PathTracer/Events.hpp>
#include <App/CameraRecorder/Events.hpp>
#include <Core/Graphics/Cuda/PathTracing/Renderer.hpp>
#include <Core/Capture/Sample.hpp>

namespace App::PathTracer
{
	class Layer : public Core::Runtime::Layer::Base
	{
	public:
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
	private:
		void OnPathTracingStart(const Events::Start& event);
		void OnPathTracingStop(const Events::Stop& event);
		void OnCameraRecordingFinish(const CameraRecorder::Events::Finish& event);
	private:
		Core::Graphics::Cuda::Renderer m_PathTracer;
		std::vector<Core::Capture::MotionState> m_RecordedFrames;
	};
}