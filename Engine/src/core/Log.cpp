#include "core/Log.hpp"

#include <iostream>

namespace Disarray {

	void Logging::Logger::debug(const std::string& message) { std::cout << message << "\n"; }
	void Logging::Logger::error(const std::string& message) { std::cerr << message << "\n"; }

} // namespace Disarray
