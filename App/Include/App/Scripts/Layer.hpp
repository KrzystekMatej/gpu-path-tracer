#pragma once
#include <Core/Runtime/Layer/Base.hpp>

namespace App::Scripts
{
	class Layer : public Core::Runtime::Layer::Base
	{
	public:
		void OnAttach() override;
	};
}