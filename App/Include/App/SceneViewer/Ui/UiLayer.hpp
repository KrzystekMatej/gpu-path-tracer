#pragma once
#include <memory>
#include <imgui.h>
#include <Core/Runtime/ImGuiLayer.hpp>
#include <App/Controller.hpp>

namespace App::SceneViewer::Ui
{
	class UiLayer : public Core::Runtime::ImGuiLayer
	{
	public:
		enum class ViewMode
		{
			LivePreview,
			PathTracedOutput
		};

		UiLayer(Controller* controller)
			: m_Controller(controller) {}

		void ImGuiBuild(const Core::Runtime::UiContext& context) override;
	private:
		ImVec2 BuildDisplay(const Core::Runtime::UiContext& context);
		float GetMaxResponsiveLabelWidth() const;
		bool ShouldExpandInputs(float panelWidth) const;
		void BuildSidePanel(const Core::Runtime::UiContext& context, const ImVec2& displaySize);
		void BuildDisplaySection(ImVec2 displaySize, const Core::Runtime::Time& time, bool expandInputs);
		void BuildCameraRecordingSection(bool expandInputs);
		void BuildPathTracingSection(bool expandInputs);
		std::expected<void, Core::Utils::Error> RecordingButtonPushed(bool recordingRunning);
		std::expected<void, Core::Utils::Error> RenderingButtonPushed(bool renderingRunning);

		Controller* m_Controller;

		bool m_LockDisplayRatio = true;
		ViewMode m_ViewMode = ViewMode::LivePreview;

		int m_TargetFps = 24;

		int m_FrameWidth = 1920;
		int m_FrameHeight = 1080;
		int m_SamplesPerPixel = 64;
		int m_PathDepth = 8;
	};
}
