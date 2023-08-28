#pragma once

#include <cstddef>
#include <vector>

#include "util/BitCast.hpp"

namespace Disarray {

class DataBuffer {
public:
	DataBuffer() = default;
	explicit DataBuffer(std::size_t);
	DataBuffer(std::nullptr_t);

	DataBuffer(const void* data, std::size_t);

	DataBuffer(const DataBuffer&);
	DataBuffer(DataBuffer&&) noexcept;
	DataBuffer& operator=(DataBuffer);
	~DataBuffer();

	void allocate(std::size_t);
	void copy_from(const DataBuffer&);

	void reset();

	template <typename T>
		requires(!std::is_same_v<T, bool>)
	T& read(std::size_t element_offset = 0)
	{
		return *bit_cast<T*>(*data + element_offset * sizeof(T));
	}

	friend void swap(DataBuffer& first, DataBuffer& second) noexcept;

	auto get_size() const { return size; }
	auto get_data() const { return *data; }

	explicit(false) operator bool() const { return is_valid(); }
	bool is_valid() const { return size != 0 && data != nullptr; }

private:
	std::unique_ptr<std::byte*> data { nullptr };
	std::size_t size { 0 };
};

template <typename T> class GenericDataBuffer {
	static constexpr auto TSize = sizeof(T);

public:
	GenericDataBuffer()
		: buffer(nullptr, TSize)
	{
	}
	explicit GenericDataBuffer(const T* data)
		: buffer(static_cast<const void*>(data), TSize)
	{
	}

private:
	DataBuffer buffer;
};

} // namespace Disarray
