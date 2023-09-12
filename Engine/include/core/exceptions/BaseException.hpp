#pragma once

#include <fmt/core.h>

#include <stdexcept>
#include <string>
#include <string_view>

namespace Disarray {

class BaseException : public std::runtime_error {
public:
	explicit BaseException(std::string_view scope, std::string_view msg);
	[[nodiscard]] auto what() const noexcept -> const char* override;

private:
	std::string message;
};

} // namespace Disarray

template <> struct fmt::formatter<Disarray::BaseException> : fmt::formatter<std::string_view> {
	auto format(const Disarray::BaseException& format, format_context& ctx) -> decltype(ctx.out());
};

template <class T>
	requires std::is_base_of_v<Disarray::BaseException, T>
struct fmt::formatter<T> : fmt::formatter<Disarray::BaseException> {
	auto format(const T& exception, format_context& ctx) -> decltype(ctx.out())
	{
		return fmt::formatter<Disarray::BaseException>::format(exception, ctx);
	}
};
