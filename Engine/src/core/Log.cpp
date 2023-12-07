#include "DisarrayPCH.hpp"

#include <fmt/chrono.h>
#include <magic_enum.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cstdarg>
#include <cstddef>
#include <vector>

#include "core/Log.hpp"
#include "core/PointerDefinition.hpp"
#include "core/exceptions/GeneralExceptions.hpp"
#include "spdlog/common.h"

namespace Disarray {

namespace Logging {

	using LoggerType = std::shared_ptr<spdlog::logger>;
	struct Logger::LoggerDataPimpl {
		spdlog::logger logger;

		explicit LoggerDataPimpl(spdlog::logger&& in_logger)
			: logger(in_logger)
		{
		}
	};

	Logger::Logger()
	{
		auto engine_logger = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		auto err_file_logger = std::make_shared<spdlog::sinks::basic_file_sink_mt>("Assets/Logs/disarray_errors.log", true);
		auto trace_file_logger = std::make_shared<spdlog::sinks::basic_file_sink_mt>("Assets/Logs/disarray_trace.log", true);
		err_file_logger->set_level(spdlog::level::trace);
		trace_file_logger->set_level(spdlog::level::trace);
		spdlog::logger logger { "Disarray", { engine_logger, err_file_logger, trace_file_logger } };
		logger_data = make_scope<LoggerDataPimpl, PimplDeleter<LoggerDataPimpl>>(std::move(logger));

		logger_data->logger.critical("Logging engine level set to: {}", spdlog::level::to_string_view(spdlog::get_level()));
	}

	auto Logger::Logger::debug(const std::string& message) -> void { logger_data->logger.debug(message); }

	auto Logger::Logger::to_file(const std::string& message) -> void { logger_data->logger.trace(message); }

	auto Logger::Logger::info(const std::string& message) -> void { logger_data->logger.info(message); }

	auto Logger::Logger::trace(const std::string& message) -> void { logger_data->logger.trace(message); }

	auto Logger::Logger::warn(const std::string& message) -> void { logger_data->logger.warn(message); }

	auto Logger::Logger::error(const std::string& message) -> void { logger_data->logger.error(message); }

	void Logger::initialise_logger(const std::string& log_level)
	{
		auto level = spdlog::level::from_str(log_level);
		spdlog::set_level(level);
	}

} // namespace Logging

template <> auto PimplDeleter<Logging::Logger::LoggerDataPimpl>::operator()(Logging::Logger::LoggerDataPimpl* ptr) noexcept -> void { delete ptr; }

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
		static constexpr auto try_format_size = 63;
		auto length = std::size_t { try_format_size };
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
