#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Window {
	public:
		virtual ~Window() = default;

		auto get_width() const { return width; }
		auto get_height() const { return height; }

		virtual bool should_close() const = 0;
		virtual void update() = 0;

	protected:
		Window(std::uint32_t w, std::uint32_t h);

	private:
		std::uint32_t width { 0 };
		std::uint32_t height { 0 };

	public:
		static Scope<Window> construct(std::uint32_t w, std::uint32_t h);
	};

} // namespace Disarray
