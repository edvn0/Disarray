#pragma once

#include <fmt/core.h>

#include <chrono>
#include <string>

#define LOG_MACRO_IMPL(type, file, line, scope, format_string, ...)                                                                                  \
	{                                                                                                                                                \
		auto fixed = fmt::format("{} at: {}:{}", scope, file, line);                                                                                 \
		Disarray::Log::Detail::type(fixed, format_string, __VA_ARGS__);                                                                              \
	}

#define DISARRAY_LOG_ERROR(scope, format, ...) LOG_MACRO_IMPL(error, __FILE__, __LINE__, scope, format, __VA_ARGS__)

#define DISARRAY_LOG_INFO(scope, format, ...) LOG_MACRO_IMPL(info, __FILE__, __LINE__, scope, format, __VA_ARGS__)

#define DISARRAY_LOG_DEBUG(scope, format, ...) LOG_MACRO_IMPL(debug, __FILE__, __LINE__, scope, format, __VA_ARGS__)

namespace Disarray {

namespace Logging {
	class Logger {
	private:
		Logger() = default;

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

	using namespace std::string_view_literals;

	static constexpr std::string_view red_begin_sv { "\033[1;31m"sv };
	static constexpr std::string_view blue_begin_sv { "\033[1;34m"sv };
	static constexpr std::string_view other_begin_sv { "\033[1;32m"sv };
	static constexpr std::string_view end_sv { "\033[0m"sv };

	static constexpr std::string_view default_format = "{}[{} - {}] {}{}";
	static constexpr std::string_view empty_scope_format = "{}[{}] {}{}";

	auto current_time(bool include_ms = true) -> std::string;
	auto format(const char* format, ...) -> std::string;

	namespace Detail {
		template <class... Args> inline void debug(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
		{
#ifdef IS_DEBUG
			const auto message = fmt::format(fmt, std::forward<Args>(args)...);
			auto formatted = fmt::format(default_format, blue_begin_sv, current_time(), scope, message, end_sv);
			Logging::Logger::logger().debug(formatted);
#endif
		}

		template <class... Args> inline void info(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
		{
			const auto message = fmt::format(fmt, std::forward<Args>(args)...);
			auto formatted = fmt::format(default_format, other_begin_sv, current_time(), scope, message, end_sv);
			Logging::Logger::logger().info(formatted);
		}

		template <class... Args> inline void error(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args)
		{
			const auto message = fmt::format(fmt, std::forward<Args>(args)...);
			auto formatted_error = fmt::format(default_format, red_begin_sv, current_time(), scope, message, end_sv);
			Logging::Logger::logger().error(formatted_error);
		}

		template <class... Args> inline void empty_debug(fmt::format_string<Args...> fmt, Args&&... args)
		{
#ifdef IS_DEBUG
			const auto message = fmt::format(fmt, std::forward<Args>(args)...);
			auto formatted = fmt::format(empty_scope_format, blue_begin_sv, current_time(), message, end_sv);
			Logging::Logger::logger().debug(formatted);
#endif
		}

		template <class... Args> inline void empty_info(fmt::format_string<Args...> fmt, Args&&... args)
		{
			const auto message = fmt::format(fmt, std::forward<Args>(args)...);
			auto formatted = fmt::format(empty_scope_format, other_begin_sv, current_time(), message, end_sv);
			Logging::Logger::logger().info(formatted);
		}

		template <class... Args> inline void empty_error(fmt::format_string<Args...> fmt, Args&&... args)
		{
			const auto message = fmt::format(fmt, std::forward<Args>(args)...);
			auto formatted_error = fmt::format(empty_scope_format, red_begin_sv, current_time(), message, end_sv);
			Logging::Logger::logger().error(formatted_error);
		}
	} // namespace Detail

} // namespace Log

} // namespace Disarray
