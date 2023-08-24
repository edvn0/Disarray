#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace Disarray {

class BaseException : public std::runtime_error {
public:
	explicit BaseException(std::string_view scope, std::string_view msg);
	const char* what() const noexcept override;

private:
	std::string message;
};

} // namespace Disarray
