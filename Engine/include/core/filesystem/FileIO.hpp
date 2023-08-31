#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string_view>

#include "core/Hashes.hpp"
#include "core/Log.hpp"
#include "util/BitCast.hpp"

namespace Disarray::FS {

namespace detail {
	template <class T, class Child> struct FileWrite {
		void write_to_file(std::string_view path, std::size_t size, std::span<T> data)
		{
			static_cast<Child&>(*this).write_to_file_impl(path, size, data);
		}
	};

	template <class T> struct GenericFileWriter : FileWrite<T, GenericFileWriter<T>> {
		auto write_to_file_impl(std::string_view path_sv, std::size_t size, std::span<T> data) -> void
		{
			std::filesystem::path path { path_sv };
			std::ofstream stream { path };
			if (!stream) {
				Log::empty_error("Could not open file: {}", path.string());
				return;
			}

			stream.write(Disarray::bit_cast<const char*>(data.data()), size);
		}
	};

} // namespace detail

template <class T> void write_to_file(std::string_view path, std::size_t size, std::span<T> data)
{
	using FW = detail::GenericFileWriter<const void*>;
	FW writer {};
	writer.write_to_file(path, size, data);
}

template <typename Func, typename ExtensionIncludeFunc, bool Recursive = false, bool IncludeDirectories = false>
auto for_each_in_directory(auto path, Func&& func, ExtensionIncludeFunc&& ext) -> void
{
	using IteratorType = std::conditional_t<Recursive, std::filesystem::directory_iterator, std::filesystem::recursive_directory_iterator>;
	using EntryType = std::filesystem::directory_entry;
	static constexpr auto should_include = [](const EntryType& entry, auto&& extension_func) -> bool {
		const auto should_include = extension_func(entry);
		if constexpr (IncludeDirectories) {
			const auto is_directory = entry.is_directory();
			return should_include && is_directory;
		} else {
			return should_include;
		}
	};

	for (const EntryType& entry : IteratorType { std::move(path) }) {
		if (!should_include(entry, std::forward<ExtensionIncludeFunc>(ext)))
			continue;

		func(entry);
	}
}

} // namespace Disarray::FS
