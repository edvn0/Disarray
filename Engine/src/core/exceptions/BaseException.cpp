#include "DisarrayPCH.hpp"

#include <algorithm>
#include <stdexcept>

#include "core/Log.hpp"
#include "core/exceptions/BaseException.hpp"

namespace Disarray {

BaseException::BaseException(std::string_view scope, std::string_view data)
	: std::runtime_error(data.data())
	, message(data)
{
	Log::error("BaseException", "An exception occurred in scope \"{}\": {}", scope, data);
}

auto BaseException::what() const noexcept -> const char* { return message.c_str(); }

} // namespace Disarray

auto fmt::formatter<Disarray::BaseException>::format(const Disarray::BaseException& format, fmt::format_context& ctx) -> decltype(ctx.out())
{
	return fmt::formatter<std::string_view>::format(fmt::format("{}", format.what()), ctx);
}
