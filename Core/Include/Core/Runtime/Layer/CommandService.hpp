#pragma once
#include <memory>
#include <Core/Utils/Guid.hpp>

namespace Core::Runtime::Layer
{
	class Base;

	class CommandService
	{
	public:
		virtual ~CommandService() = default;
		virtual Base* PushTop(Core::Utils::Guid id, std::unique_ptr<Base> layer) = 0;
		virtual Base* PushBottom(Core::Utils::Guid id, std::unique_ptr<Base> layer) = 0;
		virtual void PopTop() = 0;
		virtual void PopBottom() = 0;
		virtual bool Remove(Core::Utils::Guid id) = 0;
		virtual Base* Replace(Core::Utils::Guid id, std::unique_ptr<Base> layer) = 0;
	};
}