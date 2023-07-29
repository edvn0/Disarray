#pragma once

#include "vulkan/Image.hpp"
#include "core/DataBuffer.hpp"
#include "graphics/Texture.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

	class Texture: public Disarray::Texture {
	public:
		Texture(Ref<Device>, Ref<Swapchain>, Ref<PhysicalDevice>, const TextureProperties&);
		~Texture() override;

		void force_recreation() override { recreate(); };

		VkImageView get_view() { return image->get_view(); }

	private:
		void recreate(bool should_clean = true);

		DataBuffer pixels;

		Ref<Device> device;
		Ref<Swapchain> swapchain;
		Ref<PhysicalDevice> physical_device;
		Ref<Vulkan::Image> image;
		TextureProperties props;
	};

}