#pragma once

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
		static void debug(const std::string& scope, const std::string& message) {
			std::string formatted = "[Disarray::Engine - " + scope + "]: " + message;
			Logging::Logger::logger().debug(formatted);
		}
		static void error(const std::string& scope, const std::string& message) {
			std::string formatted = "[Disarray::Engine - " + scope + "]: " + message;
			Logging::Logger::logger().error(formatted);
		}

		std::string format(const char* const format, ...);

	} // namespace Log

} // namespace Disarray
