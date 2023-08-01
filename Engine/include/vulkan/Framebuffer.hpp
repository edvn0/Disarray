#pragma once

#include "graphics/Device.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/Texture.hpp"
#include "vulkan/PropertySupplier.hpp"
#include "vulkan/Swapchain.hpp"

#include <span>
#include <vector>

namespace Disarray::Vulkan {

	class Framebuffer : public Disarray::Framebuffer, public PropertySupplier<VkFramebuffer> {
	public:
		Framebuffer(Disarray::Device&, Disarray::Swapchain&, const FramebufferProperties&);
		~Framebuffer() override;

		void force_recreation() override;
		void recreate(bool should_clean) override { recreate_framebuffer(should_clean); }

		Disarray::RenderPass& get_render_pass() override { return *render_pass; };

		Image& get_image(std::uint32_t index) override { return textures.at(index)->get_image(); }

		VkFramebuffer supply() const override { return framebuffers[swapchain.get_image_index()]; }

	private:
		void recreate_framebuffer(bool should_clean = true);
		void create_attachment_textures();

		Disarray::Device& device;
		Disarray::Swapchain& swapchain;
		Ref<Disarray::RenderPass> render_pass;
		std::vector<VkFramebuffer> framebuffers {};

		std::vector<Ref<Disarray::Texture>> textures;
		Ref<Disarray::Texture> depth_texture;

		FramebufferProperties props;
	};

} // namespace Disarray::Vulkan
