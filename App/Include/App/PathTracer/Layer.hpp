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
		Layer() : m_DisplayTexture(Core::Graphics::Gl::GetRgba8TextureFormat()) {}

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
	private:
		void OnPathTracingStart(const Events::Start& event);
		void OnPathTracingStop(const Events::Stop& event);
		void OnCameraRecordingFinish(const CameraRecorder::Events::Finish& event);
	private:
		Core::Graphics::Cuda::Renderer m_PathTracer;
		Core::Graphics::Gl::DynamicTexture2D m_DisplayTexture;
		std::vector<Core::Capture::MotionState> m_CameraMotionStates;
		bool m_StatesUpdated = false;
	};
}