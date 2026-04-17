#pragma once
#include <Core/Runtime/Context.hpp>

namespace Core::Runtime
{
	class UiLayer
    {
    public:
        virtual ~UiLayer() = default;
        virtual void Start(const UiContext& context) {}
        virtual void BuildUi(const UiContext& context) {}
        virtual void CommitUi() {}
        virtual void Shutdown() {}
    };
}
