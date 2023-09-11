#pragma once

#include <vector>

#include "Forward.hpp"
#include "graphics/Device.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/Swapchain.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/PropertySupplier.hpp"
#include "vulkan/RenderPass.hpp"

namespace Disarray::Vulkan {

class Framebuffer : public Disarray::Framebuffer, public PropertySupplier<VkFramebuffer> {
	DISARRAY_MAKE_NONCOPYABLE(Framebuffer)
public:
	Framebuffer(const Disarray::Device&, FramebufferProperties);
	~Framebuffer() override;

	void force_recreation() override;

	void recreate(bool should_clean, const Extent& extent) override
	{
		props.extent = extent;
		recreate_framebuffer(should_clean);
		auto& cbs = get_callbacks();
		auto* this_framebuffer = this;
		for (auto& registered_callback : cbs) {
			registered_callback(*this_framebuffer);
		}
	}

	auto supply() const -> VkFramebuffer override { return framebuffer; }
	auto get_image(std::uint32_t index) -> Image& override { return *attachments.at(index); }
	auto get_depth_image() -> Disarray::Image& override { return *depth_attachment; }

	auto has_depth() -> bool override { return static_cast<bool>(depth_attachment); }
	auto get_colour_attachment_count() const -> std::uint32_t override { return colour_count; }
	auto get_render_pass() -> Disarray::RenderPass& override { return *render_pass; };
	auto get_clear_values() const -> const auto& { return clear_values; }

private:
	void recreate_framebuffer(bool should_clean = true);

	const Disarray::Device& device;
	Ref<Disarray::RenderPass> render_pass { nullptr };
	VkFramebuffer framebuffer {};
	std::vector<VkClearValue> clear_values {};

	std::uint32_t colour_count {};
	std::vector<Scope<Vulkan::Image>> attachments;
	Scope<Vulkan::Image> depth_attachment;
};

} // namespace Disarray::Vulkan
