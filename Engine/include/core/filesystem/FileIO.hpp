#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
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
		void read_from_file(std::string_view path, std::vector<T>& output) { static_cast<Child&>(*this).read_from_file_impl(path, output); }
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

	template <class T> struct GenericFileReader : FileRead<T, GenericFileReader<T>> {
		auto read_from_file_impl(std::string_view path_sv, std::vector<T>& out) -> void
		{
			std::filesystem::path path { path_sv };
			std::ifstream stream { path, std::fstream::ate | std::fstream::in };
			if (!stream) {
				Log::empty_error("Could not open file: {}", path.string());
				return;
			}

			const auto size = stream.tellg();
			out.resize(size);

			stream.seekg(0);

			char* cast = Disarray::bit_cast<char*>(out.data());
			stream.read(cast, size);
		}
	};

	using CharFileRead = GenericFileReader<char>;
	using UintFileRead = GenericFileReader<std::uint32_t>;
	using UnsignedCharFileRead = GenericFileReader<unsigned char>;
	using ConstCharFileRead = GenericFileReader<const char>;
	using ConstUintFileRead = GenericFileReader<const std::uint32_t>;
	using ConstUnsignedCharFileRead = GenericFileReader<const unsigned char>;

} // namespace Detail

template <class T> void write_to_file(std::string_view path, std::size_t size, std::span<T> data)
{
	using FW = Detail::GenericFileWriter<const void*>;
	FW writer {};
	writer.write_to_file(path, size, data);
}

template <class T>
concept AllowedVectorTypes = AnyOf<T, const char, char, const unsigned char, unsigned char, const std::uint32_t, std::uint32_t>;
template <AllowedVectorTypes T> void read_from_file(std::string_view path, std::vector<T>& output)
{
	using Reader = Detail::GenericFileReader<T>;
	Reader reader {};
	reader.read_from_file(path, output);
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
		if (!should_include(entry, std::forward<ExtensionIncludeFunc>(ext))) {
			continue;
		}

		func(entry);
	}
}

} // namespace Disarray::FS
