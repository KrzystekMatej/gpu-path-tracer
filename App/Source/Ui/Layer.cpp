#include <App/Ui/Layer.hpp>
#include <App/Ui/Utils.hpp>
#include <Core/Runtime/Application.hpp>
#include <App/CameraRecorder/Status.hpp>
#include <App/CameraRecorder/Settings.hpp>
#include <App/PathTracer/Status.hpp>
#include <App/PathTracer/Settings.hpp>
#include <App/CameraRecorder/Events.hpp>
#include <App/PathTracer/Events.hpp>

namespace App::Ui
{
	std::string_view ToString(ViewMode mode)
	{
		switch (mode)
		{
			case ViewMode::LivePreview:
				return "Live Preview";
			case ViewMode::PathTracedOutput:
				return "Path Traced Output";
			default:
				return "Unknown";
		}
	}

	void Layer::OnAttach()
	{
		Core::Runtime::Application::EventDispatcher().sink<Core::Events::KeyPressed>().connect<&Layer::OnKeyPressed>(*this);

		auto& window = Core::Runtime::Application::Window();
		m_SceneTarget = std::make_unique<Core::Graphics::Gl::RenderTarget>(window.GetWidth(), window.GetHeight());
	}

	void Layer::OnUpdate()
	{
	}

	void Layer::OnBuildUi()
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

		auto& blackboard = Core::Runtime::Application::Blackboard();
		CameraRecorder::Status& recorderStatus = blackboard.ctx().get<CameraRecorder::Status>();
		CameraRecorder::Settings& recorderSettings = blackboard.ctx().get<CameraRecorder::Settings>();
		PathTracer::Status& pathTracerStatus = blackboard.ctx().get<PathTracer::Status>();
		PathTracer::Settings& pathTracerSettings = blackboard.ctx().get<PathTracer::Settings>();
		ImVec2 displaySize = BuildDisplay();
		ImGui::SameLine();
		BuildSidePanel(displaySize);

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void Layer::OnRender(Core::Graphics::Services::SceneRenderer renderer)
	{
		Core::Graphics::Services::SceneViewDesc sceneViewDesc = {
			.target = *m_SceneTarget,
			.scene = Core::Runtime::Application::Scene(),
			.storage = Core::Runtime::Application::AssetManager().GetStorage(),
			.clearColor = { 0.1f, 0.1f, 0.1f, 1.0f }
		};

		renderer.Render(sceneViewDesc);
	}

	void Layer::OnKeyPressed(const Core::Events::KeyPressed& event)
	{
		if (!event.repeat && event.key == Core::Input::KeyCode::Escape)
			Core::Runtime::Application::Window().Close();
	}

	ImVec2 Layer::BuildDisplay()
	{
		const ImVec2 hostAvail = ImGui::GetContentRegionAvail();
		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float panelWidth = std::clamp(hostAvail.x * 0.26f, 260.0f, 360.0f);
		const float displayWidth = std::max(100.0f, hostAvail.x - panelWidth - spacing);

		ImGui::BeginChild("Display", ImVec2(displayWidth, 0.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

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

		uint32_t sceneWidth = static_cast<uint32_t>(displaySize.x);
		uint32_t sceneHeight = static_cast<uint32_t>(displaySize.y);
		if (sceneWidth != m_SceneTarget->GetWidth() || sceneHeight != m_SceneTarget->GetHeight())
			m_SceneTarget->Resize(sceneWidth, sceneHeight);

		entt::registry& blackboard = Core::Runtime::Application::Blackboard();
		PathTracer::Status& pathTracerStatus = blackboard.ctx().get<PathTracer::Status>();
		PathTracer::Settings& pathTracerSettings = blackboard.ctx().get<PathTracer::Settings>();
		pathTracerSettings.frameWidth = sceneWidth;
		pathTracerSettings.frameHeight = sceneHeight;

		uint32_t textureId = m_ViewMode == ViewMode::LivePreview ? m_SceneTarget->GetTexture().GetId() : pathTracerStatus.frameTexture->GetId();
		ImGui::Image(static_cast<ImTextureID>(textureId), displaySize, ImVec2(0, 1), ImVec2(1, 0));

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRect(cursor, ImVec2(cursor.x + displaySize.x, cursor.y + displaySize.y), IM_COL32(255, 255, 255, 50));
		drawList->AddText(ImVec2(cursor.x + 12.0f, cursor.y + 12.0f), IM_COL32(255, 255, 255, 255), ToString(m_ViewMode).data());

		ImGui::SetCursorPosY(childCursorPos.y + childAvail.y - actionsHeight);

		ImGui::BeginChild("DisplayActions", ImVec2(0.0f, actionsHeight), true);
		const float buttonsWidth = ImGui::GetContentRegionAvail().x;
		const float buttonWidth = std::max(1.0f, (buttonsWidth - style.ItemSpacing.x) * 0.5f);

		CameraRecorder::Status& recorderStatus = blackboard.ctx().get<CameraRecorder::Status>();
		const bool recordingRunning = recorderStatus.state == CameraRecorder::State::Active;
		const bool renderingRunning = pathTracerStatus.state == PathTracer::State::Active || pathTracerStatus.state == PathTracer::State::Stopping;
		const char* recordingButtonLabel = recordingRunning ? "Stop recording [R]" : "Start recording [R]";
		const char* renderingButtonLabel = renderingRunning ? "Stop rendering [T]" : "Start rendering [T]";

		if (ImGui::Button(recordingButtonLabel, ImVec2(buttonWidth, 50.0f)))
		{
			CameraRecorder::Settings& settings = blackboard.ctx().get<CameraRecorder::Settings>();
			if (recordingRunning)
			{
				Core::Runtime::Application::EventDispatcher().trigger(CameraRecorder::Events::Stop(settings));
			}
			else
			{
				CameraRecorder::Settings& settings = blackboard.ctx().get<CameraRecorder::Settings>();
				Core::Runtime::Application::EventDispatcher().trigger(CameraRecorder::Events::Start(settings));
			}
		}

		ImGui::SameLine();

		if (ImGui::Button(renderingButtonLabel, ImVec2(buttonWidth, 50.0f)))
		{
			if (renderingRunning)
			{
				Core::Runtime::Application::EventDispatcher().trigger(PathTracer::Events::Stop());
			}
			else
			{
				PathTracer::Settings& settings = blackboard.ctx().get<PathTracer::Settings>();
				Core::Runtime::Application::EventDispatcher().trigger(PathTracer::Events::Start(settings));
			}
		}

		ImGui::EndChild();

		ImGui::EndChild();
		return displaySize;
	}

	float Layer::GetMaxResponsiveLabelWidth() const
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

	bool Layer::ShouldExpandInputs(float panelContentWidth) const
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

	void Layer::BuildSidePanel(const ImVec2& displaySize)
	{
		ImGui::BeginChild("SidePanel", ImVec2(0.0f, 0.0f), true);

		const float panelContentWidth = ImGui::GetContentRegionAvail().x;
		const bool expandInputs = ShouldExpandInputs(panelContentWidth);

		BuildDisplaySection(displaySize, expandInputs);
		BuildCameraRecordingSection(expandInputs);
		BuildPathTracingSection(expandInputs);

		ImGui::EndChild();
	}

	void Layer::BuildDisplaySection(ImVec2 displaySize, bool expandInputs)
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
		entt::registry& blackboard = Core::Runtime::Application::Blackboard();

		if (ImGui::CollapsingHeader("Display", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const auto& time = Core::Runtime::Application::Time();
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

			if (Utils::BeginResponsiveCombo("View Mode", "##ViewMode", ToString(m_ViewMode).data(), expandInputs))
			{
				const ViewMode modes[] = { ViewMode::LivePreview, ViewMode::PathTracedOutput };

				for (const ViewMode mode : modes)
				{
					const bool selected = m_ViewMode == mode;

					if (ImGui::Selectable(ToString(mode).data(), selected))
						m_ViewMode = mode;

					if (selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			ImGui::EndChild();
		}
	}

	void Layer::BuildCameraRecordingSection(bool expandInputs)
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

		entt::registry& blackboard = Core::Runtime::Application::Blackboard();
		CameraRecorder::Status& recorderStatus = blackboard.ctx().get<CameraRecorder::Status>();
		CameraRecorder::Settings& recorderSettings = blackboard.ctx().get<CameraRecorder::Settings>();

		if (ImGui::CollapsingHeader("Camera Recording", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginChild("CameraRecordingStatus", ImVec2(0.0f, cameraRecordingStatusHeight), true);
			ImGui::Text("State: %s", ToString(recorderStatus.state).data());
			ImGui::Text("Recorded frames: %d", recorderStatus.doneFrames);
			ImGui::EndChild();

			ImGui::Spacing();

			ImGui::BeginChild("CameraRecordingSettings", ImVec2(0.0f, cameraRecordingSettingsHeight), true);
			Utils::BuildResponsiveInputInt("Target FPS", "##TargetFps", reinterpret_cast<int*>(&recorderSettings.targetFps), expandInputs);
			ImGui::EndChild();
		}
	}

	void Layer::BuildPathTracingSection(bool expandInputs)
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
				}
				: std::initializer_list<float>{
					frameHeight, frameHeight
				},
			style.ItemSpacing.y,
			style.WindowPadding.y);

		entt::registry& blackboard = Core::Runtime::Application::Blackboard();
		PathTracer::Status& pathTracerStatus = blackboard.ctx().get<PathTracer::Status>();
		PathTracer::Settings& pathTracerSettings = blackboard.ctx().get<PathTracer::Settings>();

		if (ImGui::CollapsingHeader("Path Tracing", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const float frameProgress = pathTracerStatus.totalFrames > 0
				? std::clamp(static_cast<float>(pathTracerStatus.doneFrames) / static_cast<float>(pathTracerStatus.totalFrames), 0.0f, 1.0f)
				: 0.0f;

			const float sampleProgress = pathTracerStatus.totalSamples > 0
				? std::clamp(static_cast<float>(pathTracerStatus.doneSamples) / static_cast<float>(pathTracerStatus.totalSamples), 0.0f, 1.0f)
				: 0.0f;

			ImGui::BeginChild("PathTracingStatus", ImVec2(0.0f, pathTracingStatusHeight), true);
			ImGui::Text("State: %s", ToString(pathTracerStatus.state).data());
			ImGui::Text("Frames: %d / %d", pathTracerStatus.doneFrames, pathTracerStatus.totalFrames);
			ImGui::ProgressBar(frameProgress, ImVec2(-1.0f, 0.0f));
			ImGui::Text("Current frame samples: %d / %d", pathTracerStatus.doneSamples, pathTracerStatus.totalSamples);
			ImGui::ProgressBar(sampleProgress, ImVec2(-1.0f, 0.0f));
			ImGui::EndChild();

			ImGui::Spacing();

			ImGui::BeginChild("PathTracingSettings", ImVec2(0.0f, pathTracingSettingsHeight), true);
			Utils::BuildResponsiveInputInt("SPP", "##SamplesPerPixel", reinterpret_cast<int*>(&pathTracerSettings.samplesPerPixel), expandInputs);
			Utils::BuildResponsiveInputInt("Path depth", "##PathDepth", reinterpret_cast<int*>(&pathTracerSettings.pathDepth), expandInputs);
			ImGui::EndChild();
		}
	}
}