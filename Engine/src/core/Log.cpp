#include "DisarrayPCH.hpp"

#include "core/Log.hpp"

#include <cstdarg>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "core/exceptions/GeneralExceptions.hpp"

namespace Disarray {

#ifdef DISARRAY_ALLOW_SLOW_LOGGING
void Logging::Logger::debug(const std::string& message) { std::cout << message << std::endl; }
void Logging::Logger::info(const std::string& message) { std::cout << message << std::endl; }
void Logging::Logger::error(const std::string& message) { std::cerr << message << std::endl; }
#else
void Logging::Logger::debug(const std::string& message) { std::cout << message << "\n"; }
void Logging::Logger::info(const std::string& message) { std::cout << message << "\n"; }
void Logging::Logger::error(const std::string& message) { std::cerr << message << "\n"; }
#endif

namespace Log {
	std::string current_time(bool include_ms)
	{
		auto now = std::chrono::system_clock::now();
		if (include_ms) {
			return fmt::format("{:%F %T}", now);
		} else {
			return fmt::format("{:%F-%T}", std::chrono::floor<std::chrono::seconds>(now));
		}
	}

	std::string format(const char* const format, ...)
	{
		auto temp = std::vector<char> {};
		auto length = std::size_t { 63 };
		std::va_list args;
		while (temp.size() <= length) {
			temp.resize(length + 1);
			va_start(args, format);
			const auto status = std::vsnprintf(temp.data(), temp.size(), format, args);
			va_end(args);
			if (status < 0) {
				throw CouldNotFormatException { "string formatting error" };
			}
			length = static_cast<std::size_t>(status);
		}
		return std::string { temp.data(), length };
	}
} // namespace Log

} // namespace Disarray
