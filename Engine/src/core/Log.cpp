#include "core/Log.hpp"

#include <iostream>

namespace Disarray {

	void Logging::Logger::debug(std::string message) { std::cout << message << "\n"; }
	void Logging::Logger::error(std::string message) { std::cerr << message << "\n"; }

} // namespace Disarray
