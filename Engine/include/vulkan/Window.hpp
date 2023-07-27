#pragma once

#include "core/Window.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/Instance.hpp"
#include "vulkan/Surface.hpp"

extern "C" {
struct GLFWwindow;
}

namespace Disarray::Vulkan {

	class Window : public Disarray::Window {
	public:
		Window(std::uint32_t, std::uint32_t);
		~Window() override;

		bool should_close() const override;
		void update() override;
		Ref<Disarray::Surface> get_surface() override { return surface; };
		Ref<Disarray::Instance> get_instance() override { return instance; };

		std::pair<int, int> get_framebuffer_size() override;

	private:
		GLFWwindow* window { nullptr };
		Ref<Surface> surface { nullptr };
		Ref<Instance> instance { nullptr };
	};

} // namespace Disarray::Vulkan
