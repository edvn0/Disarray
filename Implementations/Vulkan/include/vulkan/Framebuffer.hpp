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
public:
	Framebuffer(const Disarray::Device&, FramebufferProperties);
	~Framebuffer() override;

	void force_recreation() override;

	void recreate(bool should_clean, const Extent& extent) override
	{
		props.extent = extent;
		recreate_framebuffer(should_clean);
	}

	VkFramebuffer supply() const override { return framebuffer; }
	Image& get_image(std::uint32_t index) override { return *attachments.at(index); }
	Disarray::Image& get_depth_image() override { return *depth_attachment; }

	bool has_depth() override { return static_cast<bool>(depth_attachment); }
	std::uint32_t get_colour_attachment_count() const override { return colour_count; }
	Disarray::RenderPass& get_render_pass() override { return *render_pass; };
	const auto& get_clear_values() const { return clear_values; }

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
