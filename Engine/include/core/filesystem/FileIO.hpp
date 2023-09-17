#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>

#include "core/Concepts.hpp"
#include "core/Hashes.hpp"
#include "core/Log.hpp"
#include "core/filesystem/FileIO.hpp"
#include "util/BitCast.hpp"

namespace Disarray::FS {

namespace Detail {
	template <class T, class Child> struct FileWrite {
		void write_to_file(std::string_view path, std::size_t size, std::span<T> data)
		{
			static_cast<Child&>(*this).write_to_file_impl(path, size, data);
		}
	};

	template <class T, class Child> struct FileRead {
		auto read_from_file(std::string_view path, std::vector<T>& output) -> bool
		{
			return static_cast<Child&>(*this).read_from_file_impl(path, output);
		}

		auto read_from_file(std::string_view path, std::string& output) -> bool
		{
			return static_cast<Child&>(*this).read_from_file_impl(path, output);
		}
	};

} // namespace Detail

#include "core/filesystem/FileReadWriters.inl"

template <class T> void write_to_file(std::string_view path, std::size_t size, std::span<T> data)
{
	using FW = Detail::GenericFileWriter<T>;
	FW writer {};
	writer.write_to_file(path, size, data);
}

template <class T>
concept AllowedVectorTypes = AnyOf<T, const char, char, const unsigned char, unsigned char, const std::uint32_t, std::uint32_t>;
template <AllowedVectorTypes T> [[nodiscard]] inline auto read_from_file(std::string_view path, std::string& output) -> bool
{
	using Reader = Detail::GenericFileReader<T>;
	Reader reader {};
	return reader.read_from_file(path, output);
}

template <AllowedVectorTypes T> [[nodiscard]] auto read_from_file(std::string_view path, std::vector<T>& output) -> bool
{
	using Reader = Detail::GenericFileReader<T>;
	Reader reader {};
	return reader.read_from_file(path, output);
}

[[nodiscard]] inline auto read_from_file(std::string_view path, std::string& output) -> bool { return read_from_file<char>(path, output); }

[[nodiscard]] inline auto file_size(Pathlike auto pathlike) -> std::size_t
{
	std::ifstream opened_stream { pathlike, std::fstream::ate | std::fstream::in };
	if (!opened_stream) {
		return 0;
	}

	return static_cast<std::size_t>(opened_stream.tellg());
}

template <typename Func, typename ExtensionIncludeFunc, bool Recursive = false, bool IncludeDirectories = false>
auto for_each_in_directory(auto path, Func&& func, ExtensionIncludeFunc&& ext) -> void
{
	using IteratorType = std::conditional_t<Recursive, std::filesystem::directory_iterator, std::filesystem::recursive_directory_iterator>;
	using EntryType = std::filesystem::directory_entry;
	static constexpr auto should_include = [](const EntryType& entry, auto&& extension_func) -> bool {
		const bool should_include = extension_func(entry);
		if constexpr (IncludeDirectories) {
			const auto is_directory = entry.is_directory();
			return should_include && is_directory;
		} else {
			return should_include;
		}
	};

	for (const EntryType& entry : IteratorType { std::move(path) }) {
		if (!should_include(entry, std::forward<ExtensionIncludeFunc>(ext))) {
			continue;
		}

		std::forward<Func>(func)(entry);
	}
}

} // namespace Disarray::FS
