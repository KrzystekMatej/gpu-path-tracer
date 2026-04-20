#pragma once
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <Core/Runtime/Layer/Base.hpp>
#include <Core/Runtime/Layer/CommandService.hpp>
#include <Core/Utils/Guid.hpp>
#include <Core/Utils/Hash.hpp>

namespace Core::Runtime::Layer
{
	class CommandQueue
	{
	public:
		CommandQueue() = default;

		template <typename T, typename... Args>
		void PushTop(Core::Utils::Guid id, Args&&... args);

		template <typename T, typename... Args>
		void PushTop(std::string_view id, Args&&... args);

		template <typename T, typename... Args>
		void PushBottom(Core::Utils::Guid id, Args&&... args);

		template <typename T, typename... Args>
		void PushBottom(std::string_view id, Args&&... args);

		void PopTop();
		void PopBottom();

		void Remove(Core::Utils::Guid id);
		void Remove(std::string_view id);
		template <typename T, typename... Args>
		void Replace(Core::Utils::Guid id, Args&&... args);

		template <typename T, typename... Args>
		void Replace(std::string_view id, Args&&... args);

		void Commit(CommandService& service);
		void Clear();

	private:
		enum class CommandType
		{
			PushTop,
			PushBottom,
			PopTop,
			PopBottom,
			Remove,
			Replace
		};

		struct Command
		{
			CommandType Type = CommandType::PopTop;
			Core::Utils::Guid Id = 0;
			std::unique_ptr<Base> Instance;
		};

	private:
		static Core::Utils::Guid Hash(std::string_view id);

		template <typename T, typename... Args>
		static std::unique_ptr<Base> MakeLayer(Args&&... args);

	private:
		std::vector<Command> m_Commands;
	};

	template <typename T, typename... Args>
	void CommandQueue::PushTop(Core::Utils::Guid id, Args&&... args)
	{
		static_assert(std::is_base_of_v<Base, T>);
		Command command;
		command.Type = CommandType::PushTop;
		command.Id = id;
		command.Instance = MakeLayer<T>(std::forward<Args>(args)...);

		m_Commands.push_back(std::move(command));
	}

	template <typename T, typename... Args>
	void CommandQueue::PushTop(std::string_view id, Args&&... args)
	{
		PushTop<T>(Hash(id), std::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	void CommandQueue::PushBottom(Core::Utils::Guid id, Args&&... args)
	{
		static_assert(std::is_base_of_v<Base, T>);

		Command command;
		command.Type = CommandType::PushBottom;
		command.Id = id;
		command.Instance = MakeLayer<T>(std::forward<Args>(args)...);

		m_Commands.push_back(std::move(command));
	}

	template <typename T, typename... Args>
	void CommandQueue::PushBottom(std::string_view id, Args&&... args)
	{
		PushBottom<T>(Hash(id), std::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	void CommandQueue::Replace(Core::Utils::Guid id, Args&&... args)
	{
		static_assert(std::is_base_of_v<Base, T>);

		Command command;
		command.Type = CommandType::Replace;
		command.Id = id;
		command.Instance = MakeLayer<T>(std::forward<Args>(args)...);

		m_Commands.push_back(std::move(command));
	}

	template <typename T, typename... Args>
	void CommandQueue::Replace(std::string_view id, Args&&... args)
	{
		Replace<T>(Hash(id), std::forward<Args>(args)...);
	}

	template <typename T, typename... Args>
	std::unique_ptr<Base> CommandQueue::MakeLayer(Args&&... args)
	{
		static_assert(std::is_base_of_v<Base, T>);
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}