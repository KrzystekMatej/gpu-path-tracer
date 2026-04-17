#pragma once
#include <Core/Runtime/UiLayer.hpp>

namespace Core::Runtime
{
	class ImGuiLayer : public UiLayer
	{
	public:
		virtual void ImGuiStart(const UiContext& context) {}
		virtual void ImGuiBuild(const UiContext& context) {}
		virtual void ImGuiShutdown() {}

		void Start(const UiContext& context) override;
		void BuildUi(const UiContext& context) override;
		void CommitUi() override;
		void Shutdown() override;
	private:
		void BeginUiFrame();
	};
}