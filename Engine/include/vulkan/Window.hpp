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

		void reset_resize_status() override;
		bool was_resized() const override;

		void wait_for_minimisation() override;

		std::pair<int, int> get_framebuffer_size() override;

	private:
		struct UserData {
			bool was_resized {false};
		};
		UserData* user_data;

		GLFWwindow* window { nullptr };
		Ref<Surface> surface { nullptr };
		Ref<Instance> instance { nullptr };
	};

} // namespace Disarray::Vulkan
