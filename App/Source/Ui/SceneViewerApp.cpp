#include <App/Ui/SceneViewerApp.hpp>
#include <GLFW/glfw3.h>
#include <App/Scripts/CameraController.hpp>
#include <algorithm>
#include <cmath>
#include <App/Ui/Utils.hpp>

namespace App::Ui
{
	const char* SceneViewerApp::ToString(ViewMode mode)
	{
		switch (mode)
		{
		case ViewMode::LivePreview:
			return "Live Preview";
		case ViewMode::PathTracedOutput:
			return "Path-Traced Output";
		default:
			return "Unknown";
		}
	}

	const char* SceneViewerApp::ToString(CameraRecorderState state)
	{
		switch (state)
		{
		case CameraRecorderState::Idle:
			return "Idle";
		case CameraRecorderState::Active:
			return "Active";
		case CameraRecorderState::Stopping:
			return "Stopping";
		case CameraRecorderState::Finished:
			return "Finished";
		default:
			return "Unknown";
		}
	}

	const char* SceneViewerApp::ToString(PathTracerState state)
	{
		switch (state)
		{
		case PathTracerState::Idle:
			return "Idle";
		case PathTracerState::Ready:
			return "Ready";
		case PathTracerState::Active:
			return "Active";
		case PathTracerState::Stopping:
			return "Stopping";
		case PathTracerState::Finished:
			return "Finished";
		default:
			return "Unknown";
		}
	}

	void SceneViewerApp::ImGuiInit(const Core::Runtime::InitContext& context)
	{
		context.scriptCatalog.Register("AwakeCameraController", App::Scripts::AwakeCameraController);
		context.scriptCatalog.Register("UpdateCameraController", App::Scripts::UpdateCameraController);

		context.builderRegistry.Register("CameraController", std::make_unique<App::Scripts::CameraControllerBuilder>());

		m_Window = &context.window;
		context.eventDispatcher.sink<Core::Events::KeyPressed>().connect<&SceneViewerApp::OnKeyPressed>(*this);

		m_SceneTarget = std::make_unique<Core::Graphics::Gl::RenderTarget>(context.window.GetWidth(), context.window.GetHeight());
	}

	void SceneViewerApp::OnKeyPressed(const Core::Events::KeyPressed& event)
	{
		if (!event.repeat && event.key == Core::Input::KeyCode::Escape)
			m_Window->Close();
	}

	void SceneViewerApp::Update(const Core::Runtime::Context& context)
	{
	}

	void SceneViewerApp::ImGuiBuild(const Core::Runtime::Context& context)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags hostFlags =
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
		ImGui::Begin("MainLayout", nullptr, hostFlags);

		ImVec2 displaySize = BuildDisplay(context);
		ImGui::SameLine();
		BuildSidePanel(context, displaySize);

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void SceneViewerApp::ImGuiShutdown(const Core::Runtime::Context& context)
	{
		context.eventDispatcher.sink<Core::Events::KeyPressed>().disconnect<&SceneViewerApp::OnKeyPressed>(*this);
	}

	ImVec2 SceneViewerApp::BuildDisplay(const Core::Runtime::Context& context)
	{
		const ImVec2 hostAvail = ImGui::GetContentRegionAvail();
		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float panelWidth = std::clamp(hostAvail.x * 0.26f, 260.0f, 360.0f);
		const float displayWidth = std::max(100.0f, hostAvail.x - panelWidth - spacing);

		ImGui::BeginChild("Display", ImVec2(displayWidth, 0.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		const bool recordingRunning =
			m_CameraRecorderState == CameraRecorderState::Active ||
			m_CameraRecorderState == CameraRecorderState::Stopping;

		const bool renderingRunning =
			m_PathTracerState == PathTracerState::Active ||
			m_PathTracerState == PathTracerState::Stopping;

		const char* recordingButtonLabel = recordingRunning ? "Stop recording [R]" : "Start recording [R]";
		const char* renderingButtonLabel = renderingRunning ? "Stop rendering [T]" : "Start rendering [T]";

		const ImGuiStyle& style = ImGui::GetStyle();
		const float textHeight = ImGui::GetTextLineHeight();
		const float frameHeight = ImGui::GetFrameHeight();

		const float actionsHeight = Utils::GetStackHeight(
			{ 50 },
			style.ItemSpacing.y,
			style.WindowPadding.y
		);

		const ImVec2 childAvail = ImGui::GetContentRegionAvail();
		const ImVec2 childCursorPos = ImGui::GetCursorPos();

		ImVec2 avail = childAvail;
		avail.x = std::max(1.0f, std::floor(avail.x));
		avail.y = std::max(1.0f, std::floor(avail.y - actionsHeight - style.ItemSpacing.y));

		if (m_LockDisplayRatio)
		{
			const float targetRatio = 16.0f / 9.0f;
			const float currentRatio = avail.x / avail.y;

			if (currentRatio > targetRatio)
				avail.x = std::floor(avail.y * targetRatio);
			else
				avail.y = std::floor(avail.x / targetRatio);
		}

		const ImVec2 displaySize = avail;
		const ImVec2 cursor = ImGui::GetCursorScreenPos();

		const uint32_t sceneWidth = static_cast<uint32_t>(displaySize.x);
		const uint32_t sceneHeight = static_cast<uint32_t>(displaySize.y);

		if (sceneWidth != m_SceneTarget->GetWidth() || sceneHeight != m_SceneTarget->GetHeight())
			m_SceneTarget->Resize(sceneWidth, sceneHeight);

		Core::Graphics::Services::SceneViewDesc sceneViewDesc = {
			.target = *m_SceneTarget,
			.scene = context.scene,
			.clearColor = { 0.1f, 0.1f, 0.1f, 1.0f }
		};

		context.sceneRenderService.Render(sceneViewDesc);

		ImGui::Image(static_cast<ImTextureID>(m_SceneTarget->GetTexture().GetId()), displaySize, ImVec2(0, 1), ImVec2(1, 0));

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRect(cursor, ImVec2(cursor.x + displaySize.x, cursor.y + displaySize.y), IM_COL32(255, 255, 255, 50));
		drawList->AddText(ImVec2(cursor.x + 12.0f, cursor.y + 12.0f), IM_COL32(255, 255, 255, 255), ToString(m_ViewMode));

		ImGui::SetCursorPosY(childCursorPos.y + childAvail.y - actionsHeight);

		ImGui::BeginChild("DisplayActions", ImVec2(0.0f, actionsHeight), true);
		const float buttonsWidth = ImGui::GetContentRegionAvail().x;
		const float buttonWidth = std::max(1.0f, (buttonsWidth - style.ItemSpacing.x) * 0.5f);

		if (ImGui::Button(recordingButtonLabel, ImVec2(buttonWidth, 50.0f)))
		{
		}

		ImGui::SameLine();

		if (ImGui::Button(renderingButtonLabel, ImVec2(buttonWidth, 50.0f)))
		{
		}

		ImGui::EndChild();

		ImGui::EndChild();
		return displaySize;
	}

	float SceneViewerApp::GetMaxResponsiveLabelWidth() const
	{
		return std::max({
			ImGui::CalcTextSize("View Mode").x,
			ImGui::CalcTextSize("Target FPS").x,
			ImGui::CalcTextSize("Frame width").x,
			ImGui::CalcTextSize("Frame height").x,
			ImGui::CalcTextSize("SPP").x,
			ImGui::CalcTextSize("Path depth").x
		});
	}

	bool SceneViewerApp::ShouldExpandInputs(float panelContentWidth) const
	{
		const ImGuiStyle& style = ImGui::GetStyle();

		const float settingsChildContentWidth = std::max(
			1.0f,
			panelContentWidth
				- 2.0f * style.ChildBorderSize
				- 2.0f * style.WindowPadding.x
		);

		const float widestLabelWidth = GetMaxResponsiveLabelWidth();
		const float minFieldWidth = 170.0f;
		const float safetyMargin = 12.0f;

		const float requiredInlineWidth =
			widestLabelWidth +
			style.ItemInnerSpacing.x +
			minFieldWidth +
			safetyMargin;

		return settingsChildContentWidth < requiredInlineWidth;
	}

	void SceneViewerApp::BuildSidePanel(const Core::Runtime::Context& context, const ImVec2& displaySize)
	{
		ImGui::BeginChild("SidePanel", ImVec2(0.0f, 0.0f), true);

		const float panelContentWidth = ImGui::GetContentRegionAvail().x;
		const bool expandInputs = ShouldExpandInputs(panelContentWidth);

		BuildDisplaySection(displaySize, context.time, expandInputs);
		BuildCameraRecordingSection(expandInputs);
		BuildPathTracingSection(expandInputs);

		ImGui::EndChild();
	}

	void SceneViewerApp::BuildDisplaySection(ImVec2 displaySize, const Core::Runtime::Time& time, bool expandInputs)
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const float textHeight = ImGui::GetTextLineHeight();
		const float frameHeight = ImGui::GetFrameHeight();

		const float displayStatusHeight = Utils::GetStackHeight(
			{ textHeight, textHeight, textHeight, textHeight, textHeight },
			style.ItemSpacing.y,
			style.WindowPadding.y);

		const float displaySettingsHeight = Utils::GetStackHeight(
			expandInputs
				? std::initializer_list<float>{ frameHeight, textHeight, frameHeight }
				: std::initializer_list<float>{ frameHeight, frameHeight },
			style.ItemSpacing.y,
			style.WindowPadding.y);

		const float ratio = displaySize.y > 0.0f ? displaySize.x / displaySize.y : 0.0f;

		if (ImGui::CollapsingHeader("Display", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginChild("DisplayStatus", ImVec2(0.0f, displayStatusHeight), true);
			ImGui::Text("Width: %.0f px", displaySize.x);
			ImGui::Text("Height: %.0f px", displaySize.y);
			ImGui::Text("Aspect: %.3f", ratio);
			ImGui::Text("FPS: %.1f", time.GetFps());
			ImGui::Text("Frame time: %.2f ms", time.GetDeltaTimeMs());
			ImGui::EndChild();

			ImGui::Spacing();

			ImGui::BeginChild("DisplaySettings", ImVec2(0.0f, displaySettingsHeight), true);
			ImGui::Checkbox("Lock to 16:9", &m_LockDisplayRatio);

			if (Utils::BeginResponsiveCombo("View Mode", "##ViewMode", ToString(m_ViewMode), expandInputs))
			{
				const ViewMode modes[] = { ViewMode::LivePreview, ViewMode::PathTracedOutput };

				for (const ViewMode mode : modes)
				{
					const bool selected = m_ViewMode == mode;

					if (ImGui::Selectable(ToString(mode), selected))
						m_ViewMode = mode;

					if (selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			ImGui::EndChild();
		}
	}

	void SceneViewerApp::BuildCameraRecordingSection(bool expandInputs)
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const float textHeight = ImGui::GetTextLineHeight();
		const float frameHeight = ImGui::GetFrameHeight();

		const float cameraRecordingStatusHeight = Utils::GetStackHeight(
			{ textHeight, textHeight },
			style.ItemSpacing.y,
			style.WindowPadding.y);

		const float cameraRecordingSettingsHeight = Utils::GetStackHeight(
			expandInputs
				? std::initializer_list<float>{ textHeight, frameHeight }
				: std::initializer_list<float>{ frameHeight },
			style.ItemSpacing.y,
			style.WindowPadding.y);

		if (ImGui::CollapsingHeader("Camera Recording", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginChild("CameraRecordingStatus", ImVec2(0.0f, cameraRecordingStatusHeight), true);
			ImGui::Text("State: %s", ToString(m_CameraRecorderState));
			ImGui::Text("Recorded frames: %d", m_RecordedFrames);
			ImGui::EndChild();

			ImGui::Spacing();

			ImGui::BeginChild("CameraRecordingSettings", ImVec2(0.0f, cameraRecordingSettingsHeight), true);
			Utils::BuildResponsiveInputInt("Target FPS", "##TargetFps", &m_TargetFps, expandInputs);
			m_TargetFps = std::max(m_TargetFps, 1);
			ImGui::EndChild();
		}
	}

	void SceneViewerApp::BuildPathTracingSection(bool expandInputs)
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const float textHeight = ImGui::GetTextLineHeight();
		const float frameHeight = ImGui::GetFrameHeight();

		const float pathTracingStatusHeight = Utils::GetStackHeight(
			{ textHeight, textHeight, frameHeight, textHeight, frameHeight },
			style.ItemSpacing.y,
			style.WindowPadding.y);

		const float pathTracingSettingsHeight = Utils::GetStackHeight(
			expandInputs
				? std::initializer_list<float>{
					textHeight, frameHeight,
					textHeight, frameHeight,
					textHeight, frameHeight,
					textHeight, frameHeight
				}
				: std::initializer_list<float>{
					frameHeight, frameHeight, frameHeight, frameHeight
				},
			style.ItemSpacing.y,
			style.WindowPadding.y);

		if (ImGui::CollapsingHeader("Path Tracing", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const float frameProgress = m_TotalFrames > 0
				? std::clamp(static_cast<float>(m_DoneFrames) / static_cast<float>(m_TotalFrames), 0.0f, 1.0f)
				: 0.0f;

			const float sampleProgress = m_TotalSamples > 0
				? std::clamp(static_cast<float>(m_DoneSamples) / static_cast<float>(m_TotalSamples), 0.0f, 1.0f)
				: 0.0f;

			ImGui::BeginChild("PathTracingStatus", ImVec2(0.0f, pathTracingStatusHeight), true);
			ImGui::Text("State: %s", ToString(m_PathTracerState));
			ImGui::Text("Frames: %d / %d", m_DoneFrames, m_TotalFrames);
			ImGui::ProgressBar(frameProgress, ImVec2(-1.0f, 0.0f));
			ImGui::Text("Current frame samples: %d / %d", m_DoneSamples, m_TotalSamples);
			ImGui::ProgressBar(sampleProgress, ImVec2(-1.0f, 0.0f));
			ImGui::EndChild();

			ImGui::Spacing();

			ImGui::BeginChild("PathTracingSettings", ImVec2(0.0f, pathTracingSettingsHeight), true);
			Utils::BuildResponsiveInputInt("Frame width", "##FrameWidth", &m_FrameWidth, expandInputs);
			Utils::BuildResponsiveInputInt("Frame height", "##FrameHeight", &m_FrameHeight, expandInputs);
			Utils::BuildResponsiveInputInt("SPP", "##SamplesPerPixel", &m_SamplesPerPixel, expandInputs);
			Utils::BuildResponsiveInputInt("Path depth", "##PathDepth", &m_PathDepth, expandInputs);

			m_SamplesPerPixel = std::max(m_SamplesPerPixel, 1);
			m_FrameWidth = std::max(m_FrameWidth, 1);
			m_FrameHeight = std::max(m_FrameHeight, 1);
			m_PathDepth = std::max(m_PathDepth, 1);
			ImGui::EndChild();
		}
	}
}
