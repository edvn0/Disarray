#pragma once

#include "graphics/BufferProperties.hpp"
#include "graphics/IndexBuffer.hpp"
#include "vulkan/BaseBuffer.hpp"

namespace Disarray::Vulkan {

class IndexBuffer : public Disarray::IndexBuffer {
public:
	IndexBuffer(const Disarray::Device& dev, BufferProperties properties);
	~IndexBuffer() override = default;
	auto size() const -> std::size_t override;
	void set_data(const void*, std::size_t size, std::size_t offset) override;
	void set_data(const void*, std::size_t size) override;

	auto supply() const -> VkBuffer;
	auto count() const -> std::size_t override;
	auto get_raw() -> void* override;
	auto get_raw() const -> void* override;

private:
	Vulkan::BaseBuffer base_buffer;
};

} // namespace Disarray::Vulkan
