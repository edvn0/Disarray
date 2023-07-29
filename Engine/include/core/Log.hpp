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
		static void debug(const std::string& message) { Logging::Logger::logger().debug(message); }
		static void error(const std::string& message) { Logging::Logger::logger().error(message); }

		std::string format(const char* const format, ...);

	} // namespace Log

} // namespace Disarray
