#pragma once

#include "ImageProperties.hpp"
#include "core/DataBuffer.hpp"

#include <string>

namespace Disarray {

class ImageLoader {
public:
	explicit ImageLoader(const std::filesystem::path&, DataBuffer&);
	~ImageLoader();

	const auto& get_extent() const { return extent; }

private:
	void free();

	Extent extent;
	void* data;
	std::uint64_t size;
};

} // namespace Disarray
