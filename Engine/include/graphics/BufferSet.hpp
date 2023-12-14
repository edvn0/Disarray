#pragma once

#include "Forward.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

#include "core/Ensure.hpp"
#include "core/PointerDefinition.hpp"
#include "graphics/BufferProperties.hpp"
#include "graphics/Device.hpp"
#include "graphics/RendererProperties.hpp"
#include "graphics/Swapchain.hpp"

namespace Disarray {

template <typename T>
concept BufferLike = requires(T& t, const Device& device) {
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

namespace Detail {
	auto get_current_frame(const Disarray::IGraphicsResource* graphics_resource) -> FrameIndex;
}

template <BufferLike B> class BufferSet {
public:
	explicit BufferSet(const Device* dev, const Disarray::IGraphicsResource* graphics_resource = nullptr, std::uint32_t frames = 0)
		: device(dev)
		, graphics_resource(graphics_resource)
		, frame_count(frames)
	{
	}
	~BufferSet() = default;

	auto set_frame_count(std::uint32_t frames, const Disarray::IGraphicsResource* resource) -> void
	{
		ensure(frame_set_binding_buffers.empty(), "BufferSet must be initialized before setting frame count");
		ensure(frames > 0, "BufferSet must be initialized with a frame count greater than 0");
		ensure(resource != nullptr, "BufferSet must be initialized with a valid IGraphicsResource");
		frame_count = frames;
		graphics_resource = resource;
	}

	auto create(std::integral auto size, DescriptorBinding binding, std::size_t count = 0U) -> void
	{
		for (auto frame = FrameIndex(0); frame < frame_count; ++frame) {
			auto buffer = B::construct(*device,
				BufferProperties {
					.size = static_cast<std::size_t>(size),
					.count = count,
					.binding = binding.value,
					.always_mapped = true,
				});
			set(std::move(buffer), frame);
		}
	}

	auto get(DescriptorBinding binding, DescriptorSet set = DescriptorSet { 0 }) -> auto&
	{
		return frame_set_binding_buffers.at(current_frame()).at(set).at(binding);
	}

	auto get(DescriptorBinding binding, FrameIndex frame_index, DescriptorSet set) -> auto&
	{
		return frame_set_binding_buffers.at(frame_index).at(set).at(binding);
	}

	auto set(Ref<B>&& buffer, FrameIndex frame_index, DescriptorSet set = DescriptorSet { 0 }) -> void
	{
		frame_set_binding_buffers[frame_index][set].try_emplace(buffer->get_binding(), std::move(buffer));
	}

private:
	const Device* device;
	const IGraphicsResource* graphics_resource;

	auto current_frame() const -> FrameIndex { return Detail::get_current_frame(graphics_resource); }

	std::uint32_t frame_count { 0 };
	using BindingBuffers = std::unordered_map<DescriptorBinding, Ref<B>>;
	using SetBindingBuffers = std::unordered_map<DescriptorSet, BindingBuffers>;

	std::unordered_map<FrameIndex, SetBindingBuffers> frame_set_binding_buffers {};
};

} // namespace Disarray
