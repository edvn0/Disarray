#pragma once

#include "Forward.hpp"
#include "ImageProperties.hpp"
#include "core/Types.hpp"
#include "core/Window.hpp"
#include "graphics/Device.hpp"
#include "graphics/RenderPass.hpp"

namespace Disarray {

class Framebuffer;

class Swapchain {
public:
	static Scope<Disarray::Swapchain> construct(Disarray::Window&, Disarray::Device&, Disarray::Swapchain* = nullptr);

	virtual std::uint32_t image_count() const = 0;
	virtual Extent get_extent() const = 0;
	virtual SampleCount get_samples() const = 0;

	virtual std::uint32_t get_current_frame() const = 0;
	virtual std::uint32_t advance_frame() = 0;
	virtual std::uint32_t get_image_index() const = 0;

	virtual bool prepare_frame() = 0;
	virtual void present() = 0;

	virtual bool needs_recreation() const = 0;
	virtual void reset_recreation_status() = 0;

	virtual Disarray::RenderPass& get_render_pass() = 0;

	virtual ~Swapchain() = default;
};

} // namespace Disarray
