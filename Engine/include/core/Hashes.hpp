#pragma once

#include <filesystem>
#include <functional>
#include <string_view>

namespace Disarray {

struct StringHash {
	using is_transparent = void;

	std::size_t operator()(std::string_view sv) const
	{
		std::hash<std::string_view> hasher;
		return hasher(sv);
	}
};

struct string_hash {
	using is_transparent = void;
	[[nodiscard]] size_t operator()(const char* txt) const { return std::hash<std::string_view> {}(txt); }
	[[nodiscard]] size_t operator()(std::string_view txt) const { return std::hash<std::string_view> {}(txt); }
	[[nodiscard]] size_t operator()(const std::string& txt) const { return std::hash<std::string> {}(txt); }
};

struct FileSystemPathHasher {
	std::size_t operator()(const std::filesystem::path& path) const
	{
		std::hash<std::filesystem::path::string_type> hasher;
		return hasher(path.native());
	}
};

} // namespace Disarray
