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
		static constexpr std::string_view end_sv { "\033[0m"sv };
		static std::string red_begin { red_begin_sv };
		static std::string blue_begin { blue_begin_sv };
		static std::string end { end_sv };

		inline std::string current_time() { return fmt::format("{:%F %T}", std::chrono::system_clock::now()); }

		inline void debug(const std::string& scope, const std::string& message)
		{
#ifdef IS_DEBUG
			std::string formatted = blue_begin + "[" + current_time() + " - Disarray::Engine - " + scope + "]: " + message + end;
			Logging::Logger::logger().debug(formatted);
#endif
		}

		inline void error(const std::string& scope, const std::string& message)
		{
			std::string formatted = red_begin + "[" + current_time() + " - Disarray::Engine - " + scope + "]: " + message + end;
			Logging::Logger::logger().error(formatted);
		}

		std::string format(const char* const format, ...);

	} // namespace Log

} // namespace Disarray
