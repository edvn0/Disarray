#pragma once

#include "graphics/RenderPass.hpp"
#include "vulkan/PropertySupplier.hpp"
#include "vulkan/Swapchain.hpp"
#include "graphics/Framebuffer.hpp"

#include <span>
#include <vector>
#include "graphics/Texture.hpp"

namespace Disarray::Vulkan {

	class Framebuffer: public Disarray::Framebuffer, public PropertySupplier<VkFramebuffer>
	{
	public:
		Framebuffer(Ref<Disarray::Device>, Ref<Disarray::Swapchain>, Ref<Disarray::PhysicalDevice> physical_device,Ref<Disarray::RenderPass>,  const FramebufferProperties&);
		~Framebuffer() override;

		void force_recreation() override;

		VkFramebuffer supply() const override { return framebuffers[swapchain->get_image_index()]; }

	private:
		void recreate(bool should_clean = true);

		Ref<Device> device;
		Ref<Vulkan::Swapchain> swapchain;
		Ref<Disarray::RenderPass> render_pass;
		std::vector<VkFramebuffer> framebuffers {};
		Ref<Disarray::Texture> depth_texture;
		FramebufferProperties props;
	};

}