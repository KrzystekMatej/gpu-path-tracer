#pragma once
#include <spdlog/spdlog.h>
#include <memory>
#include <string>

namespace Core
{
	class Error
	{
	public:
		Error() = default;

		Error(std::string message)
				: m_Message(std::move(message)) {}
		
		Error(std::string message, Error source)
			: m_Message(std::move(message)),
			  m_Source(std::make_shared<Error>(std::move(source))) {}

		Error(std::string message, std::shared_ptr<Error> source)
			: m_Message(std::move(message)),
			  m_Source(std::move(source)) {}

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
