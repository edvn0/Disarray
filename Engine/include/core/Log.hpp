#pragma once

#include <chrono>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <string>

namespace Disarray {

	namespace Logging {
		class Logger {
		private:
			Logger() = default;
			~Logger() = default;

		public:
			void debug(const std::string&);
			void info(const std::string&);
			void error(const std::string&);

			static Logger& logger()
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

		std::string current_time(bool include_ms = true);

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

		std::string format(const char* const format, ...);

	} // namespace Log

} // namespace Disarray
