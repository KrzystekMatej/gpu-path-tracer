#pragma once
#include <memory>
#include <Core/Runtime/Context.hpp>

namespace Core::Runtime
{
	class UiClient
	{
	public:
		virtual ~UiClient() = default;
		virtual void Init(const InitContext& context) = 0;
		virtual void Update(const Context& context) = 0;
		virtual void BuildUi(const Context& context) = 0;
		virtual void CommitUi() = 0;
		virtual void Shutdown(const Context& context) = 0;
	};
}