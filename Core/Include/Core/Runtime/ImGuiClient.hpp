#pragma once
#include <Core/Runtime/UiClient.hpp>

namespace Core::Runtime
{
	class ImGuiClient : public UiClient
	{
	public:
		virtual void ImGuiInit(const InitContext& context) = 0;
		virtual void ImGuiBuild(const Context& context) = 0;
		virtual void ImGuiShutdown() = 0;

		void Init(const InitContext& context) override;
		void BuildUi(const Context& context) override;
		void CommitUi() override;
		void Shutdown() override;
	private:
		void BeginUiFrame();
	};
}