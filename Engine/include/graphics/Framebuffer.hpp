#pragma once

#include "Forward.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "graphics/Device.hpp"
#include "graphics/Image.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/Swapchain.hpp"

namespace Disarray {

	struct FramebufferProperties {
		ImageFormat format { ImageFormat::SBGR };
		std::uint32_t colour_count { 1 };
		ImageFormat depth_format { ImageFormat::Depth };
		SampleCount samples { SampleCount::ONE };
		Extent extent { 0, 0 };
		bool load_colour { false };
		bool keep_colour { true };
		bool load_depth { false };
		bool keep_depth { true };
		bool has_depth { true };
		bool should_present { false };
		Ref<Disarray::RenderPass> optional_renderpass { nullptr };
		std::string debug_name { "UnknownFramebuffer" };
	};

	class Framebuffer : public ReferenceCountable {
		DISARRAY_OBJECT(Framebuffer)
	public:
		virtual Disarray::Image& get_image(std::uint32_t index) = 0;
		Disarray::Image& get_image() { return get_image(0); };

		virtual Disarray::RenderPass& get_render_pass() = 0;

		virtual std::uint32_t get_colour_attachment_count() = 0;
		virtual bool has_depth() = 0;

		static Ref<Framebuffer> construct(Disarray::Device&, const FramebufferProperties&);
	};

} // namespace Disarray
