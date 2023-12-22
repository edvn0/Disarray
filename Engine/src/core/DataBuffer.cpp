#include "DisarrayPCH.hpp"

#include <cstring>
#include <utility>

#include "core/DataBuffer.hpp"
#include "core/Log.hpp"

namespace Disarray {

DataBuffer::DataBuffer(std::size_t s)
	: data(std::make_unique<std::byte[]>(s))
	, size(s)
{
}

void DataBuffer::zero_initialise() const
{
	// Write zeroes into data
	std::memset(data.get(), 0U, size);
}

DataBuffer::DataBuffer(std::nullptr_t) { }

DataBuffer::~DataBuffer() { }

void DataBuffer::reset()
{
	data.reset();
	size = 0;
}

void DataBuffer::copy_from(const DataBuffer& buffer)
{
	if (!buffer.is_valid()) {
		return;
	}

	reset();
	size = buffer.size;
	data = std::make_unique<std::byte[]>(size);
	std::memcpy(data.get(), buffer.data.get(), size);
}

void DataBuffer::allocate(std::size_t s)
{
	reset();
	size = s;
	data = std::make_unique<std::byte[]>(size);
}

DataBuffer::DataBuffer(const DataBuffer& other) { copy_from(other); }

DataBuffer::DataBuffer(DataBuffer&& other) noexcept
	: data(std::move(other.data))
	, size(other.size)
{
}

auto DataBuffer::operator=(const DataBuffer& other) -> DataBuffer&
{
	const auto new_size = other.size;
	if (new_size != size) {
		allocate(new_size);
	}
	std::memcpy(data.get(), other.data.get(), new_size);

	return *this;
}

auto DataBuffer::operator=(DataBuffer&& other) noexcept -> DataBuffer&
{
	swap(*this, other);

	return *this;
}

void swap(DataBuffer& first, DataBuffer& second) noexcept
{
	using std::swap;

	swap(first.size, second.size);
	swap(first.data, second.data);
}

} // namespace Disarray
