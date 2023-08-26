#pragma once

#include <cstddef>
#include <span>
#include <string_view>

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
				Log::empty_error("Could not open file: {}", path);
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

} // namespace Disarray::FS
