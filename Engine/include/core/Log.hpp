#pragma once

#include <fmt/core.h>

#include <chrono>
#include <string>

#include "core/Types.hpp"

namespace Disarray {

namespace Logging {

	struct LoggerDataPimpl;

	class Logger {
	private:
		Logger();

		Scope<LoggerDataPimpl, PimplDeleter<LoggerDataPimpl>> logger_data;

	public:
		void trace(const std::string&);
		void debug(const std::string&);
		void info(const std::string&);
		void warn(const std::string&);
		void error(const std::string&);
		void to_file(const std::string&);

		static void initialise_logger(const std::string& log_level);

		static auto the() -> Logger&
		{
			static Logger logger;
			return logger;
		}
	};
} // namespace Logging

namespace Log {

	auto current_time(bool include_ms = true) -> std::string;
	auto format(const char* format, ...) -> std::string;

	template <class... Args> inline void to_file(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
	{
		const auto message = fmt::format(fmt, std::forward<Args>(args)...);
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::the().to_file(formatted);
	}

	template <class... Args> inline void trace(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
	{
		const auto message = fmt::format(fmt, std::forward<Args>(args)...);
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::the().trace(formatted);
	}

	template <class... Args> inline void debug(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
	{
		const auto message = fmt::format(fmt, std::forward<Args>(args)...);
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::the().debug(formatted);
	}

	template <class... Args> inline void info(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
	{
		const auto message = fmt::format(fmt, std::forward<Args>(args)...);
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::the().info(formatted);
	}

	template <class... Args> inline void warn(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
	{
		const auto message = fmt::format(fmt, std::forward<Args>(args)...);
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::the().warn(formatted);
	}

	template <class... Args> inline void error(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
	{
		const auto message = fmt::format(fmt, std::forward<Args>(args)...);
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::the().error(formatted);
	}

	inline void to_file(std::string_view scope, std::string_view message)
	{
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::the().to_file(formatted);
	}

	inline void trace(std::string_view scope, std::string_view message)
	{
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::the().trace(formatted);
	}

	inline void debug(std::string_view scope, std::string_view message)
	{
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::the().debug(formatted);
	}

	inline void info(std::string_view scope, std::string_view message)
	{
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::the().info(formatted);
	}

	inline void warn(std::string_view scope, std::string_view message)
	{
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::the().warn(formatted);
	}

	inline void error(std::string_view scope, std::string_view message)
	{
		auto formatted = fmt::format("[{}] {}", scope, message);
		Logging::Logger::the().error(formatted);
	}

} // namespace Log

} // namespace Disarray
