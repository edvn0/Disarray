#pragma once

#include "Forward.hpp"
#include "graphics/Framebuffer.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/PropertySupplier.hpp"
#include "vulkan/RenderPass.hpp"

#include <span>
#include <vector>

namespace Disarray::Vulkan {

	class Framebuffer : public Disarray::Framebuffer, public PropertySupplier<VkFramebuffer> {
	public:
		Framebuffer(Disarray::Device&, const FramebufferProperties&);
		~Framebuffer() override;

		void force_recreation() override;

		void recreate(bool should_clean, const Extent& extent) override
		{
			props.extent = extent;
			recreate_framebuffer(should_clean);
		}

		VkFramebuffer supply() const override { return framebuffer; }

		bool has_depth() override { return static_cast<bool>(depth_attachment); }
		Image& get_image(std::uint32_t index) override { return *attachments.at(index); }
		std::uint32_t get_colour_attachment_count() const override { return colour_count; }
		Disarray::RenderPass& get_render_pass() override { return *render_pass; };
		const auto& get_clear_values() const { return clear_values; }

		const FramebufferProperties& get_properties() const override { return props; }
		FramebufferProperties& get_properties() override { return props; }

	private:
		void recreate_framebuffer(bool should_clean = true);

		std::uint32_t colour_count {};

		Disarray::Device& device;
		Ref<Disarray::RenderPass> render_pass { nullptr };
		VkFramebuffer framebuffer {};
		std::vector<VkClearValue> clear_values {};

		std::vector<Scope<Vulkan::Image>> attachments;
		Scope<Vulkan::Image> depth_attachment;

		FramebufferProperties props;
	};

} // namespace Disarray::Vulkan
