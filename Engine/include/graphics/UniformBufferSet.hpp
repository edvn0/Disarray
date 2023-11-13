#pragma once

#include "graphics/BufferProperties.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/UniformBuffer.hpp"

namespace Disarray {

template <class BufferFor> class UniformBufferSet {
	using UniformBufferScopeVector = std::vector<Scope<Disarray::UniformBuffer>>;
	using ForwardConstIterator = UniformBufferScopeVector::const_iterator;
	using ForwardIterator = UniformBufferScopeVector::iterator;

public:
	class BufferUpdateTransaction {
	public:
		explicit BufferUpdateTransaction(UniformBufferSet<BufferFor>& set)
			: buffer_set(set)
		{
		}
		~BufferUpdateTransaction() { commit(); }

		void commit()
		{
			if (has_commited) {
				return;
			}
			buffer_set.update();
		};

		auto get_buffer() -> BufferFor& { return buffer_set.get_buffer(); }

	private:
		UniformBufferSet<BufferFor>& buffer_set;
		bool has_commited { false };
	};

	explicit UniformBufferSet(const Disarray::Device& dev, FrameIndex frames, BufferProperties properties)
		: device(dev)
		, frame_count(frames)
		, props(properties)
	{
		buffers.resize(frame_count.value);
		for (auto& buffer : buffers) {
			buffer = UniformBuffer::construct_scoped(device,
				{
					.data = props.data,
					.size = props.size,
					.count = props.count,
					.always_mapped = props.always_mapped,
				});
		}
	}

	auto transaction() -> BufferUpdateTransaction { return BufferUpdateTransaction { *this }; }

	auto read() const -> const BufferFor& { return data; }

	auto operator[](FrameIndex index) -> decltype(auto) { return buffers.at(index.value); }
	auto operator[](std::integral auto index) -> decltype(auto) { return buffers.at(index); }

	[[nodiscard]] auto begin() const -> ForwardConstIterator { return std::begin(buffers); }
	[[nodiscard]] auto end() const -> ForwardConstIterator { return std::end(buffers); }

	void update()
	{
		for (auto& buffer : buffers) {
			buffer->set_data<BufferFor>(&data);
		}
	};

private:
	const Device& device;
	FrameIndex frame_count {};
	BufferProperties props {};

	BufferFor data {};
	auto get_buffer() -> BufferFor& { return data; }

	UniformBufferScopeVector buffers {};
};

} // namespace Disarray
