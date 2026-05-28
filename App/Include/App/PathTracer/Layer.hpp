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
		Layer() : m_DisplayTexture(std::move(Core::Graphics::Gl::Texture2D::CreateRgba8())) {}

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
	private:
		void OnSceneChanged(const Core::Ecs::SceneChangedEvent& event);
		void OnPathTracingStart(const Events::Start& event);
		void OnPathTracingStop(const Events::Stop& event);
		void OnCameraRecordingFinish(const CameraRecorder::Events::Finish& event);
	private:
		Core::Graphics::Cuda::Renderer m_PathTracer;
		Core::Graphics::Gl::Texture2D m_DisplayTexture;
		std::vector<Core::Capture::MotionState> m_CameraMotionStates;
		bool m_StatesUpdated = false;
		bool m_SceneBuffersDirty = true;
	};
}