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
	[[nodiscard]] auto operator()(const char* txt) const -> std::size_t { return std::hash<std::string_view> {}(txt); }
	[[nodiscard]] auto operator()(std::string_view txt) const -> std::size_t { return std::hash<std::string_view> {}(txt); }
	[[nodiscard]] auto operator()(const std::filesystem::path& txt) const -> std::size_t { return std::hash<std::string_view> {}(txt.string()); }
	[[nodiscard]] auto operator()(const std::string& txt) const -> std::size_t { return std::hash<std::string> {}(txt); }
};

struct FileSystemPathHasher {
	auto operator()(const std::filesystem::path& path) const -> std::size_t
	{
		std::hash<std::filesystem::path::string_type> hasher;
		return hasher(path.native());
	}
};

} // namespace Disarray
