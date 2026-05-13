#pragma once
#include <expected>
#include <spdlog/spdlog.h>
#include <memory>
#include <string>
#include <format>
#include <utility>

namespace Core::Utils
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

#define CORE_TRY(name, expression)                                			\
	auto name##_expected = (expression);                           			\
	if (!name##_expected) {                                        			\
		return std::unexpected(std::move(name##_expected).error()); 		\
	}                                                             			\
	auto name = std::move(name##_expected).value()

#define CORE_TRY_CONTEXT(name, expression, ...)                             \
	auto name##_expected = (expression);                                    \
	if (!name##_expected) {                                                 \
		return std::unexpected(                                             \
			Core::Utils::Error(                                             \
				std::make_shared<Core::Utils::Error>(                       \
					std::move(name##_expected).error()                      \
				),                                                          \
				__VA_ARGS__                                                 \
			)                                                               \
		);                                                                  \
	}                                                                       \
	auto name = std::move(name##_expected).value()

#define CORE_TRY_DISCARD(expression)                                 			\
	do {                                                          			\
		auto core_try_expected = (expression);                     			\
		if (!core_try_expected) {                                 			\
			return std::unexpected(std::move(core_try_expected).error()); 	\
		}                                                         			\
	} while (false)

#define CORE_TRY_DISCARD_CONTEXT(expression, ...)                              \
	do {                                                          			\
		auto core_try_expected = (expression);                     			\
		if (!core_try_expected) {                                 			\
			return std::unexpected(                                         \
				Core::Utils::Error(                                         \
					std::make_shared<Core::Utils::Error>(                   \
						std::move(core_try_expected).error()                \
					),                                                      \
					__VA_ARGS__                                             \
				)                                                           \
			);                                                              \
		}																	\
	} while (false)