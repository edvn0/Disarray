#pragma once

#include "core/Window.hpp"
#include "vulkan/PropertySupplier.hpp"
#include "graphics/Swapchain.hpp"

namespace Disarray::Vulkan {
	class Swapchain: public Disarray::Swapchain, public PropertySupplier<VkSwapchainKHR> {
	public:
		Swapchain(Scope<Disarray::Window>&, Ref<Disarray::Device>, Ref<Disarray::PhysicalDevice>);
		~Swapchain() override;

		VkSwapchainKHR get() const override { return swapchain; }

	private:
		Ref<Disarray::Device> device;
		VkSwapchainKHR swapchain;
	};
}