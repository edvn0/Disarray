#pragma once

#include "core/Types.hpp"
#include "graphics/Instance.hpp"
#include "graphics/Surface.hpp"

#include <utility>

namespace Disarray {

	struct WindowProperties {
		std::uint32_t width { 0 };
		std::uint32_t height { 0 };
		std::string name {};
		bool is_fullscreen { false };
	};

	class Window {
	public:
		virtual ~Window() = default;

		const WindowProperties& get_properties();

		virtual bool should_close() const = 0;
		virtual void update() = 0;
		virtual Surface& get_surface() = 0;
		virtual Instance& get_instance() = 0;

		virtual bool was_resized() const = 0;
		virtual void reset_resize_status() = 0;

		virtual void wait_for_minimisation() = 0;

		virtual void* native() = 0;

		virtual std::pair<int, int> get_framebuffer_size() = 0;
		virtual std::pair<float, float> get_framebuffer_scale() = 0;

	protected:
		Window(const WindowProperties&);

	private:
		WindowProperties props;

	public:
		static Scope<Disarray::Window> construct(const WindowProperties&);
	};

} // namespace Disarray
