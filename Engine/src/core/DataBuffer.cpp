#include "core/DataBuffer.hpp"

#include "core/Log.hpp"

#include <cstring>
#include <utility>

namespace Disarray {

	DataBuffer::DataBuffer(std::size_t s)
		: size(s)
	{
		data = new std::byte[size];
	}

	DataBuffer::DataBuffer(const void* new_data, std::size_t s)
		: DataBuffer(s)
	{
		std::memcpy(data, new_data, s);
	}

	DataBuffer::DataBuffer(std::nullptr_t) { }

	DataBuffer::~DataBuffer() { delete[] data; }

	void DataBuffer::reset()
	{
		delete[] data;
		size = 0;
	}

	void DataBuffer::copy_from(const DataBuffer& buffer)
	{
		reset();
		size = buffer.size;
		data = new std::byte[size];
		std::memcpy(data, buffer.data, size);
	}

	void DataBuffer::allocate(std::size_t s)
	{
		reset();
		size = s;
		data = new std::byte[size];
	}

	DataBuffer::DataBuffer(const DataBuffer& other) { copy_from(other); }

	DataBuffer::DataBuffer(DataBuffer&& other) noexcept
		: data(std::move(other.data))
		, size(other.size)
	{
	}

	DataBuffer& DataBuffer::operator=(DataBuffer other)
	{
		swap(*this, other); // (2)

		return *this;
	}

	void swap(DataBuffer& first, DataBuffer& second) noexcept
	{
		using std::swap;

		swap(first.size, second.size);
		swap(first.data, second.data);
	}

} // namespace Disarray
