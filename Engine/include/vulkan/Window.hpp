#pragma once

#include "core/Window.hpp"
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

	private:
		GLFWwindow* window { nullptr };
		Ref<Surface> surface { nullptr };
		Ref<Instance> instance { nullptr };
	};

} // namespace Disarray::Vulkan
