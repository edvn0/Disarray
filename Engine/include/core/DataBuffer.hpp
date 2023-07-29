#pragma once

#include <cstddef>
#include <vector>

namespace Disarray {

	class DataBuffer {
	public:
		explicit DataBuffer(std::size_t);
		DataBuffer(const void* data, std::size_t);

		DataBuffer(const DataBuffer&);
		DataBuffer(DataBuffer&&) noexcept;
		DataBuffer& operator=(DataBuffer);
		~DataBuffer();

		void allocate(std::size_t);
		void copy_from(const DataBuffer&);

		void reset();

		template<typename T> requires (not std::is_same_v<T, bool>)
		T& read(std::size_t element_offset = 0) {
			return *std::bit_cast<T*>(data + element_offset * sizeof(T));
		}

		friend void swap(DataBuffer& first, DataBuffer& second);

	private:
		std::byte* data {};
		std::size_t size {0};
	};

}
