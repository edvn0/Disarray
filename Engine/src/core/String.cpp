#include "core/String.hpp"

namespace Disarray::StringUtilities {

namespace Detail {
	auto split_string(const std::string& input, const std::string& delimiter, bool correct_slashes) -> std::vector<std::string>
	{
		std::vector<std::string> split_strings;
		std::size_t pos = 0;
		std::size_t found;

		std::string actual_delimiter = delimiter;
		if (delimiter.find("/") != std::string::npos || delimiter.find("\\") != std::string::npos) {
			// replace all slashes in parameter delimiter with the correct slash
			std::ranges::replace(actual_delimiter, '/', correct_slashes ? '\\' : '/');
			std::ranges::replace(actual_delimiter, '\\', correct_slashes ? '\\' : '/');
		}

		while ((found = input.find(actual_delimiter, pos)) != std::string::npos) {
			if (auto token = input.substr(pos, found - pos); !token.empty()) {
				split_strings.push_back(token);
			}
			pos = found + actual_delimiter.length();
		}

		if (const auto last_token = input.substr(pos); !last_token.empty()) {
			split_strings.push_back(last_token);
		}

		return split_strings;
	}

	auto human_readable_size(std::size_t size) -> std::string
	{
		using namespace std::string_view_literals;
		static constexpr auto suffixes = std::array { "B"sv, "KB"sv, "MB"sv, "GB"sv, "TB"sv, "PB"sv, "EB"sv, "ZB"sv, "YB"sv };
		static constexpr auto size_suffixes = sizeof(suffixes) / sizeof(suffixes[0]) - 1;

		int suffix_index = 0;
		auto double_size = static_cast<double>(size);

		static constexpr auto megabyte = 1024.0;

		while (double_size >= megabyte && suffix_index < size_suffixes) {
			double_size /= megabyte;
			suffix_index++;
		}

		return fmt::format("{:.2f}{}", double_size, suffixes[suffix_index]);
	}
} // namespace Detail

} // namespace Disarray::StringUtilities
