#pragma once
#include <spdlog/spdlog.h>
#include <memory>
#include <string>
#include <format>
#include <utility>

namespace Core
{
	class Error
	{
	public:
		Error() = default;

		Error(std::string message)
			: m_Message(std::move(message)) {}

		Error(std::shared_ptr<Error> source, std::string message)
			: m_Message(std::move(message)),
			  m_Source(std::move(source)) {}

		template<typename... Args>
		explicit Error(std::format_string<Args...> fmt, Args&&... args)
			: m_Message(std::format(fmt, std::forward<Args>(args)...)) {}

		template<typename... Args>
		Error(std::shared_ptr<Error> source, std::format_string<Args...> fmt, Args&&... args)
			: m_Message(std::format(fmt, std::forward<Args>(args)...)), m_Source(std::move(source)) {}

		const std::string& Message() const
		{
			return m_Message;
		}

		const std::shared_ptr<Error>& Source() const
		{
			return m_Source;
		}
		
		std::string FullMessage() const {
			if (!m_Source) {
				return m_Message;
			}

			return m_Message + "\nCaused by: " + m_Source->FullMessage();
		}

		void Log(spdlog::level::level_enum level = spdlog::level::err) const
		{
			spdlog::log(level, "{}", FullMessage());
		}

	private:
		std::string m_Message;
		std::shared_ptr<Error> m_Source;
	};
}
