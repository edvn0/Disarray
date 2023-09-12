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

		static void initialise_logger(const std::string& log_level);

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
		const auto message = fmt::format(fmt, std::forward<Args>(args)...);
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::logger().debug(formatted);
	}

	template <class... Args> inline void info(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
	{
		const auto message = fmt::format(fmt, std::forward<Args>(args)...);
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::logger().info(formatted);
	}

	template <class... Args> inline void error(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
	{
		const auto message = fmt::format(fmt, std::forward<Args>(args)...);
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::logger().error(formatted);
	}

	inline void debug(std::string_view scope, std::string_view message)
	{
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::logger().debug(formatted);
	}

	inline void info(std::string_view scope, std::string_view message)
	{
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::logger().info(formatted);
	}

	inline void error(std::string_view scope, std::string_view message)
	{
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::logger().error(formatted);
	}

} // namespace Log

} // namespace Disarray
