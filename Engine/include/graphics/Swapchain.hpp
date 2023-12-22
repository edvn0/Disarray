#pragma once

#include "Forward.hpp"

#include "core/Window.hpp"
#include "graphics/Device.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/RendererProperties.hpp"

namespace Disarray {

class Swapchain {
public:
	static auto construct(Disarray::Window&, Disarray::Device&, Disarray::Swapchain* = nullptr) -> Scope<Disarray::Swapchain>;

	[[nodiscard]] virtual auto image_count() const -> std::uint32_t = 0;
	[[nodiscard]] virtual auto get_extent() const -> Extent = 0;
	[[nodiscard]] virtual auto get_samples() const -> SampleCount = 0;

	[[nodiscard]] virtual auto get_current_frame() const -> std::uint32_t = 0;
	[[nodiscard]] auto get_current_frame_index() const -> FrameIndex { return FrameIndex { get_current_frame() }; };
	[[nodiscard]] virtual auto get_image_index() const -> std::uint32_t = 0;
	virtual auto advance_frame() -> std::uint32_t = 0;

	virtual auto prepare_frame() -> bool = 0;
	virtual void present() = 0;

	[[nodiscard]] virtual auto needs_recreation() const -> bool = 0;
	virtual void reset_recreation_status() = 0;

	virtual auto get_render_pass() -> Disarray::RenderPass& = 0;

	virtual ~Swapchain() = default;
};

} // namespace Disarray
