#include <Core/Runtime/ImGuiClient.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace Core::Runtime
{
	void ImGuiClient::Init(const InitContext& context)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui_ImplGlfw_InitForOpenGL(context.window.GetRawHandle(), true);
		ImGui_ImplOpenGL3_Init();
		ImGuiInit(context);
	}

	void ImGuiClient::BuildUi(const Context& context)
	{
		BeginUiFrame();
		ImGuiBuild(context);
	}

	void ImGuiClient::CommitUi()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void ImGuiClient::Shutdown(const Context& context)
	{
		ImGuiShutdown(context);
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiClient::BeginUiFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
}