#pragma once

#include <fmt/core.h>

#include <chrono>
#include <string>

#include "core/Types.hpp"

namespace Disarray {

namespace Logging {
	class Logger {
	private:
		Logger();

		struct LoggerDataPimpl;
		Scope<LoggerDataPimpl, PimplDeleter<LoggerDataPimpl>> logger_data;

	public:
		void debug(const std::string&);
		void info(const std::string&);
		void error(const std::string&);

		static auto logger() -> Logger&
		{
			static Logger logger;
			return logger;
		}
	};
} // namespace Logging

namespace Log {

	auto current_time(bool include_ms = true) -> std::string;
	auto format(const char* format, ...) -> std::string;

	template <class... Args> inline void debug(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
	{
#ifdef IS_DEBUG
		const auto message = fmt::format(fmt, std::forward<Args>(args)...);
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::logger().debug(std::move(formatted));
#endif
	}

	template <class... Args> inline void info(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
	{
		const auto message = fmt::format(fmt, std::forward<Args>(args)...);
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::logger().info(std::move(formatted));
	}

	template <class... Args> inline void error(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
	{
		const auto message = fmt::format(fmt, std::forward<Args>(args)...);
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::logger().error(std::move(formatted));
	}

} // namespace Log

} // namespace Disarray
