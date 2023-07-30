#pragma once

#include "core/DataBuffer.hpp"
#include <string>

namespace Disarray {

	class ImageLoader {
	public:
		explicit ImageLoader(const std::string&, DataBuffer&);
		~ImageLoader();
		void free();
	private:
		void* data;
		std::uint64_t size;
	};

}
