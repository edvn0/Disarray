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
	Log::error(scope, "{}", data);
}

const char* BaseException::what() const noexcept { return message.c_str(); }

} // namespace Disarray