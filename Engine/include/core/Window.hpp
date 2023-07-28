#pragma once

#include "core/Types.hpp"
#include "graphics/Instance.hpp"
#include "graphics/Surface.hpp"
#include <utility>

namespace Disarray {

	class Window {
	public:
		virtual ~Window() = default;

		auto get_width() const { return width; }
		auto get_height() const { return height; }

		virtual bool should_close() const = 0;
		virtual void update() = 0;
		virtual Ref<Surface> get_surface() = 0;
		virtual Ref<Instance> get_instance() = 0;

		virtual bool was_resized() const = 0;
		virtual void reset_resize_status() = 0;

		virtual void wait_for_minimisation() = 0;

		virtual std::pair<int, int> get_framebuffer_size() = 0;

	protected:
		Window(std::uint32_t w, std::uint32_t h);

	private:
		std::uint32_t width { 0 };
		std::uint32_t height { 0 };

	public:
		static Scope<Window> construct(std::uint32_t w, std::uint32_t h);
	};

} // namespace Disarray
