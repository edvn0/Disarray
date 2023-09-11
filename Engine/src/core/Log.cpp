#include "DisarrayPCH.hpp"

#include "core/Log.hpp"

#include <fmt/chrono.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cstdarg>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "core/PointerDefinition.hpp"
#include "core/exceptions/GeneralExceptions.hpp"

namespace Disarray {

template <> auto PimplDeleter<Logging::Logger::LoggerDataPimpl>::operator()(Logging::Logger::LoggerDataPimpl* ptr) noexcept -> void
{
	operator delete(ptr);
}

namespace Logging {

	using LoggerType = std::shared_ptr<spdlog::logger>;
	struct Logger::LoggerDataPimpl {
		LoggerType engine_logger { nullptr };
		LoggerType std_err_logger { nullptr };
	};

	Logger::Logger()
	{
		logger_data = make_scope<LoggerDataPimpl, PimplDeleter<LoggerDataPimpl>>();

		logger_data->engine_logger = spdlog::stdout_color_mt("Disarray");
		logger_data->std_err_logger = spdlog::stderr_color_mt("Error");
	}

	auto Logger::Logger::debug(const std::string& message) -> void { logger_data->engine_logger->debug(message); }

	auto Logger::Logger::info(const std::string& message) -> void { logger_data->engine_logger->info(message); }

	auto Logger::Logger::error(const std::string& message) -> void
	{
		logger_data->engine_logger->error(message);
		logger_data->std_err_logger->error(message);
	}

} // namespace Logging

namespace Log {
	auto current_time(bool include_ms) -> std::string
	{
		auto now = std::chrono::system_clock::now();
		if (include_ms) {
			return fmt::format("{:%F %T}", now);
		}

		return fmt::format("{:%F-%T}", std::chrono::floor<std::chrono::seconds>(now));
	}

	auto format(const char* const format, ...) -> std::string
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
