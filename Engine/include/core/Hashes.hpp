#pragma once

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

#define MAKE_HASHABLE(type, ...)                                                                                                                     \
	namespace std {                                                                                                                                  \
		template <> struct hash<type> {                                                                                                              \
			std::size_t operator()(const type& t) const                                                                                              \
			{                                                                                                                                        \
				std::size_t ret = 0;                                                                                                                 \
				hash_combine(ret, __VA_ARGS__);                                                                                                      \
				return ret;                                                                                                                          \
			}                                                                                                                                        \
		};                                                                                                                                           \
	}

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
