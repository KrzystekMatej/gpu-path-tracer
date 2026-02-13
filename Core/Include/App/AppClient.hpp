#pragma once
#include <memory>

namespace Core
{
	class AppClient
	{
	public:
		virtual void Update() = 0;
		virtual void Render() = 0;
	};
}