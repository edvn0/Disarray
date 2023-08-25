#pragma once

#include <string>

#include "ImageProperties.hpp"
#include "core/DataBuffer.hpp"

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
