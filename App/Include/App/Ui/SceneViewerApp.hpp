#pragma once
#include <memory>
#include <Core/Runtime/ImGuiClient.hpp>
#include <Core/Events/Keyboard.hpp>
#include <Core/Graphics/Gl/RenderTarget.hpp>
#include "imgui.h"

namespace App::Ui
{
	class SceneViewerApp : public Core::Runtime::ImGuiClient
	{
	public:
		void ImGuiInit(const Core::Runtime::InitContext& context) override;
		void Update(const Core::Runtime::Context& context) override;
		void ImGuiBuild(const Core::Runtime::Context& context) override;
		void ImGuiShutdown(const Core::Runtime::Context& context) override;

	private:
		enum class ViewMode
		{
			LivePreview,
			PathTracedOutput
		};

		enum class CameraRecorderState
		{
			Idle,
			Active,
			Stopping,
			Finished
		};

		enum class PathTracerState
		{
			Idle,
			Ready,
			Active,
			Stopping,
			Finished
		};

		static const char* ToString(ViewMode mode);
		static const char* ToString(CameraRecorderState state);
		static const char* ToString(PathTracerState state);

		ImVec2 BuildDisplay(const Core::Runtime::Context& context);
		float GetMaxResponsiveLabelWidth() const;
		bool ShouldExpandInputs(float panelWidth) const;
		void BuildSidePanel(const Core::Runtime::Context& context, const ImVec2& displaySize);
		void BuildDisplaySection(ImVec2 displaySize, const Core::Runtime::Time& time, bool expandInputs);
		void BuildCameraRecordingSection(bool expandInputs);
		void BuildPathTracingSection(bool expandInputs);
		void OnKeyPressed(const Core::Events::KeyPressed& event);

		Core::Window::NativeWindow* m_Window = nullptr;
		std::unique_ptr<Core::Graphics::Gl::RenderTarget> m_SceneTarget;

		bool m_LockDisplayRatio = true;
		ViewMode m_ViewMode = ViewMode::LivePreview;

		CameraRecorderState m_CameraRecorderState = CameraRecorderState::Idle;
		int m_RecordedFrames = 0;
		int m_TargetFps = 24;

		PathTracerState m_PathTracerState = PathTracerState::Idle;
		int m_DoneFrames = 0;
		int m_TotalFrames = 240;
		int m_DoneSamples = 0;
		int m_TotalSamples = 64;

		int m_FrameWidth = 1920;
		int m_FrameHeight = 1080;
		int m_SamplesPerPixel = 64;
		int m_PathDepth = 8;
	};
}
