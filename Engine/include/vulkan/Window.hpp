#pragma once

#include "core/Window.hpp"
#include "vulkan/Instance.hpp"
#include "vulkan/Surface.hpp"
#include "vulkan/Swapchain.hpp"

extern "C" {
struct GLFWwindow;
}

namespace Disarray::Vulkan {

	class Window : public Disarray::Window {
	public:
		Window(const Disarray::ApplicationProperties&);
		~Window() override;

		bool should_close() const override;
		void update() override;
		Disarray::Surface& get_surface() override { return *surface; };
		Disarray::Instance& get_instance() override { return *instance; };

		void reset_resize_status() override;
		bool was_resized() const override;

		void wait_for_minimisation() override;

		void* native() override { return window; }

		std::pair<int, int> get_framebuffer_size() override;
		std::pair<float, float> get_framebuffer_scale() override;

	private:
		struct UserData {
			bool was_resized { false };
		};
		UserData* user_data;

		GLFWwindow* window { nullptr };
		Scope<Instance> instance { nullptr };
		Scope<Surface> surface { nullptr };
	};

} // namespace Disarray::Vulkan
