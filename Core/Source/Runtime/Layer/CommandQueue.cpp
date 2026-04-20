#include <Core/Runtime/Layer/CommandQueue.hpp>

namespace Core::Runtime::Layer
{
	void CommandQueue::PopTop()
	{
		Command command;
		command.Type = CommandType::PopTop;
		m_Commands.push_back(std::move(command));
	}

	void CommandQueue::PopBottom()
	{
		Command command;
		command.Type = CommandType::PopBottom;
		m_Commands.push_back(std::move(command));
	}

	void CommandQueue::Remove(Core::Utils::Guid id)
	{
		Command command;
		command.Type = CommandType::Remove;
		command.Id = id;
		m_Commands.push_back(std::move(command));
	}

	void CommandQueue::Remove(std::string_view id)
	{
		Remove(Hash(id));
	}

	void CommandQueue::Commit(CommandService& service)
	{
		for (auto& command : m_Commands)
		{
			switch (command.Type)
			{
			case CommandType::PushTop:
				service.PushTop(command.Id, std::move(command.Instance));
				break;

			case CommandType::PushBottom:
				service.PushBottom(command.Id, std::move(command.Instance));
				break;

			case CommandType::PopTop:
				service.PopTop();
				break;

			case CommandType::PopBottom:
				service.PopBottom();
				break;

			case CommandType::Remove:
				service.Remove(command.Id);
				break;

			case CommandType::Replace:
				service.Replace(command.Id, std::move(command.Instance));
				break;
			}
		}

		m_Commands.clear();
	}

	void CommandQueue::Clear()
	{
		m_Commands.clear();
	}

	Core::Utils::Guid CommandQueue::Hash(std::string_view id)
	{
		return Core::Utils::Hasher::MakeId(id);
	}
}