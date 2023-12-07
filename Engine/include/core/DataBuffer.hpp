#pragma once

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

#include "util/BitCast.hpp"

namespace Disarray {

class DataBuffer {
public:
	DataBuffer() = default;
	explicit DataBuffer(std::integral auto input_size)
		: DataBuffer(nullptr, static_cast<std::size_t>(input_size))
	{
	}
	DataBuffer(const void* data, std::integral auto input_size)
		: DataBuffer(static_cast<std::size_t>(input_size))
	{
		std::memcpy(this->data.get(), data, size);
	}

	explicit DataBuffer(std::nullptr_t);

	DataBuffer(const DataBuffer&);
	DataBuffer(DataBuffer&&) noexcept;
	auto operator=(DataBuffer) -> DataBuffer&;
	auto operator=(DataBuffer&&) noexcept -> DataBuffer&;
	~DataBuffer();

	void allocate(std::size_t);
	void copy_from(const DataBuffer&);
	template <class T> auto read(const std::size_t offset) const -> T&
	{
		T* result = nullptr;
		std::memcpy(result, get_data() + offset, sizeof(T));
		return *result;
	}

	void write(const auto& data, auto size, auto offset) { std::memcpy(get_data() + offset, &data, size); }
	void write(const auto& data, auto size) { std::memcpy(get_data(), &data, size); }
	void write(const auto& data) { std::memcpy(get_data(), &data, sizeof(data)); }
	void zero_initialise() const;

	void reset();

	friend void swap(DataBuffer& first, DataBuffer& second) noexcept;

	[[nodiscard]] auto get_size() const -> std::size_t { return size; }
	[[nodiscard]] auto get_data() const { return data.get(); }

	explicit(false) operator bool() const { return is_valid(); }
	[[nodiscard]] auto is_valid() const -> bool { return size != 0 && data != nullptr; }

	auto operator[](std::size_t index) -> decltype(auto) { return data[index]; }

	auto as_span() -> std::span<std::byte> { return { data.get(), size }; }

private:
	explicit DataBuffer(std::size_t);

	std::unique_ptr<std::byte[]> data { nullptr };
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
