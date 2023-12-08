#include "core/String.hpp"

namespace Disarray::StringUtilities {

namespace Detail {
	auto split_string(const std::string& input, const std::string& delimiter) -> std::vector<std::string>
	{
		std::vector<std::string> split_strings;
		std::size_t pos = 0;
		std::size_t found;

		while ((found = input.find(delimiter, pos)) != std::string::npos) {
			if (auto token = input.substr(pos, found - pos); !token.empty()) {
				split_strings.push_back(token);
			}
			pos = found + delimiter.length();
		}

		if (const auto last_token = input.substr(pos); !last_token.empty()) {
			split_strings.push_back(last_token);
		}

		return split_strings;
	}
} // namespace Detail

} // namespace Disarray::StringUtilities
