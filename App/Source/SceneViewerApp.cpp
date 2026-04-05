#include "SceneViewerApp.hpp"
#include <GLFW/glfw3.h>
#include "Scripting/CameraController.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace App
{
    void SceneViewerApp::ImGuiInit(const Core::App::InitContext& context)
    {
        context.scriptCatalog.Register("AwakeCameraController", App::Scripting::AwakeCameraController);
        context.scriptCatalog.Register("UpdateCameraController", App::Scripting::UpdateCameraController);

        context.resolverRegistry.RegisterResolver("CameraController", std::make_unique<App::Scripting::CameraControllerResolver>());

        m_Window = &context.window;
        context.eventDispatcher.sink<Core::Events::KeyPressed>().connect<&SceneViewerApp::OnKeyPressed>(*this);

		m_SceneTarget = std::make_unique<Core::Graphics::Gl::RenderTarget>(context.window.GetWidth(), context.window.GetHeight());
    }

	void SceneViewerApp::OnKeyPressed(const Core::Events::KeyPressed& event)
	{
		if (!event.repeat && event.key == Core::Input::KeyCode::Escape)
		{
			m_Window->Close();
		}
	}

	void SceneViewerApp::Update(const Core::App::Context& context)
	{
	}

	void SceneViewerApp::ImGuiBuild(const Core::App::Context& context)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags host_flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
        ImGui::Begin("MainLayout", nullptr, host_flags);

        const ImVec2 hostAvail = ImGui::GetContentRegionAvail();
        const float spacing = ImGui::GetStyle().ItemSpacing.x;
        const float panelWidth = std::clamp(hostAvail.x * 0.26f, 260.0f, 360.0f);
        const float displayWidth = std::max(100.0f, hostAvail.x - panelWidth - spacing);

        ImGui::BeginChild("Display", ImVec2(displayWidth, 0.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImVec2 avail = ImGui::GetContentRegionAvail();
        avail.x = std::max(1.0f, std::floor(avail.x));
        avail.y = std::max(1.0f, std::floor(avail.y));

        if (m_LockDisplayRatio)
        {
            const float target = 16.0f / 9.0f;
            const float current = avail.x / avail.y;

            if (current > target)
                avail.x = std::floor(avail.y * target);
            else
                avail.y = std::floor(avail.x / target);
        }


        ImVec2 displaySize = avail;
		const ImVec2 cursor = ImGui::GetCursorScreenPos();
        uint32_t sceneWidth = static_cast<uint32_t>(displaySize.x);
        uint32_t sceneHeight = static_cast<uint32_t>(displaySize.y);
        if (displayWidth != m_SceneTarget->GetWidth() || sceneHeight != m_SceneTarget->GetHeight())
			m_SceneTarget->Resize(displayWidth, sceneHeight);

        Core::Graphics::SceneViewDesc sceneViewDesc = {
            .target = *m_SceneTarget,
            .scene = context.scene,
            .clearColor = { 0.1f, 0.1f, 0.1f, 1.0f }
        };
        context.sceneRenderService.Render(sceneViewDesc);
		ImGui::Image(static_cast<ImTextureID>(m_SceneTarget->GetTexture().GetId()), displaySize, ImVec2(0, 1), ImVec2(1, 0));

		if (m_ShowOverlay)
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->AddRect(cursor, ImVec2(cursor.x + displaySize.x, cursor.y + displaySize.y), IM_COL32(255, 255, 255, 50));
			draw_list->AddText(ImVec2(cursor.x + 12.0f, cursor.y + 12.0f), IM_COL32(255, 255, 255, 255), "Scene display");
		}

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("SidePanel", ImVec2(0.0f, 0.0f), true);

        const float ratio = displaySize.y > 0.0f ? displaySize.x / displaySize.y : 0.0f;

        ImGui::TextUnformatted("Side panel");
        ImGui::Separator();
        ImGui::Text("Display width:  %.0f px", displaySize.x);
        ImGui::Text("Display height: %.0f px", displaySize.y);
        ImGui::Text("Display ratio:  %.3f", ratio);
        ImGui::Spacing();

        ImGui::Checkbox("Overlay", &m_ShowOverlay);
        ImGui::Checkbox("Lock display to 16:9", &m_LockDisplayRatio);
        ImGui::SliderFloat("Clear strength", &m_ClearStrength, 0.25f, 2.0f);
        ImGui::Combo("Mode", &m_SelectedMode, "Scene\0Lighting\0Debug\0Post\0");
        ImGui::Separator();
        ImGui::TextUnformatted("Example controls");
        ImGui::Button("Action A", ImVec2(-1.0f, 0.0f));
        ImGui::Button("Action B", ImVec2(-1.0f, 0.0f));
        ImGui::Button("Action C", ImVec2(-1.0f, 0.0f));

        ImGui::EndChild();

        ImGui::End();
        ImGui::PopStyleVar();
	}

	void SceneViewerApp::ImGuiShutdown()
	{
	}
}