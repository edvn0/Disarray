#pragma once

#include "util/BitCast.hpp"

#include <cstddef>
#include <string_view>

namespace Disarray::FS {

void write_to_file(std::string_view, std::size_t, const void*);

template <class T> static void write_to_file(std::string_view path, std::size_t size, const T* data)
{
	FS::write_to_file(std::move(path), size, Disarray::bit_cast<const void*>(data));
}

} // namespace Disarray::FS
