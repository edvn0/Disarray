#include "DisarrayPCH.hpp"

#include "core/exceptions/BaseException.hpp"

#include <algorithm>
#include <stdexcept>

#include "core/Log.hpp"

namespace Disarray {

BaseException::BaseException(std::string_view scope, std::string_view data)
	: std::runtime_error(data.data())
	, message(data)
{
	DISARRAY_LOG_ERROR(scope, "{}", message);
}

auto BaseException::what() const noexcept -> const char* { return message.c_str(); }

} // namespace Disarray
