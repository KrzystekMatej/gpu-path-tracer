#pragma once
#include <memory>
#include "AppContext.hpp"
#include "Events/Event.hpp"

namespace Core
{
	class AppClient
	{
	public:
		virtual void OnEvent(const AppContext& context, const Event& event) = 0;
		virtual void Update(const AppContext& context) = 0;
		virtual void Render(const AppContext& context) = 0;
	};
}