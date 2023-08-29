#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <string_view>

namespace Disarray {

inline void hash_combine(std::size_t& seed) { }

template <typename T, typename... Rest> inline void hash_combine(std::size_t& seed, const T& v, Rest... rest)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	hash_combine(seed, rest...);
}

struct StringHash {
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
