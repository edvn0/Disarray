#pragma once

#include "Forward.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

#include "core/Ensure.hpp"
#include "core/PointerDefinition.hpp"
#include "graphics/BufferProperties.hpp"
#include "graphics/RendererProperties.hpp"
#include "graphics/Swapchain.hpp"

namespace Disarray {

template <typename T>
concept BufferLike = requires(T t, const Device& device) {
	{
		t.size()
	} -> std::convertible_to<std::size_t>;
	{
		t.get_binding()
	} -> std::convertible_to<DescriptorBinding>;
	{
		t.get_raw()
	} -> std::convertible_to<const void*>;
	{
		T::construct(device, BufferProperties {})
	} -> std::convertible_to<Ref<T>>;
};

template <BufferLike B> class BufferSet {
public:
	explicit BufferSet(const Device& dev, std::uint32_t frames = 0)
		: device(dev)
		, frame_count(frames)
	{
	}
	~BufferSet() = default;

	auto set_frame_count(std::uint32_t frames) -> void
	{
		ensure(frame_set_binding_buffers.empty(), "BufferSet must be initialized before setting frame count");
		ensure(frames > 0, "BufferSet must be initialized with a frame count greater than 0");
		frame_count = frames;
	}

	auto create(std::integral auto size, DescriptorBinding binding) -> void
	{
		ensure(frame_count > 0, "BufferSet must be initialized with a frame count greater than 0");
		ensure(frame_set_binding_buffers.empty(), "BufferSet must be initialized before creating buffers");
		for (auto frame = FrameIndex(0); frame < frame_count; ++frame) {
			auto buffer = B::construct(device,
				BufferProperties {
					.size = static_cast<std::size_t>(size),
					.binding = binding.value,
					.always_mapped = true,
				});
			set(std::move(buffer), DescriptorSet(0), frame);
		}
	}

	auto get(DescriptorBinding binding, DescriptorSet set = DescriptorSet { 0 }, FrameIndex frame = FrameIndex { 0 }) -> Ref<B>
	{
		return frame_set_binding_buffers.at(frame).at(set).at(binding);
	}

	auto set(Ref<B>&& buffer, DescriptorSet set = DescriptorSet { 0 }, FrameIndex frame = FrameIndex { 0 }) -> void
	{
		frame_set_binding_buffers[frame][set][buffer->get_binding()] = buffer;
	}

private:
	const Device& device;
	std::uint32_t frame_count { 0 };
	using BindingBuffers = std::unordered_map<DescriptorBinding, Ref<B>>;
	using SetBindingBuffers = std::unordered_map<DescriptorSet, BindingBuffers>;

	std::unordered_map<FrameIndex, SetBindingBuffers> frame_set_binding_buffers { 0 };
};

} // namespace Disarray
