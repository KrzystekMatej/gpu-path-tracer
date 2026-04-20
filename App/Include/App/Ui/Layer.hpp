#pragma once
#include <Core/Runtime/Layer/Base.hpp>
#include <App/PathTracer/Status.hpp>
#include <App/PathTracer/Settings.hpp>
#include <App/CameraRecorder/Status.hpp>
#include <App/CameraRecorder/Settings.hpp>
#include <Core/Graphics/Gl/RenderTarget.hpp>
#include <Core/Events/Keyboard.hpp>
#include "imgui.h"

namespace App::Ui
{
	enum class ViewMode
	{
		LivePreview,
		PathTracedOutput
	};

	std::string_view ToString(ViewMode mode);

	class Layer : public Core::Runtime::Layer::Base
	{
	public:
		void OnAttach() override;
		void OnUpdate() override;
		void OnBuildUi() override;
		void OnRender(Core::Graphics::Services::SceneRenderer renderer) override;
	private:
		ImVec2 BuildDisplay();
		float GetMaxResponsiveLabelWidth() const;
		bool ShouldExpandInputs(float panelWidth) const;
		void BuildSidePanel(const ImVec2& displaySize);
		void BuildDisplaySection(ImVec2 displaySize, bool expandInputs);
		void BuildCameraRecordingSection(bool expandInputs);
		void BuildPathTracingSection(bool expandInputs);

		void OnKeyPressed(const Core::Events::KeyPressed& event);

		bool m_LockDisplayRatio = true;
		ViewMode m_ViewMode = ViewMode::LivePreview;

		std::unique_ptr<Core::Graphics::Gl::RenderTarget> m_SceneTarget;
	};
}