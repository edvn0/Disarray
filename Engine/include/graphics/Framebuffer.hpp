#pragma once

#include "Forward.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

	struct FramebufferProperties {
		ImageFormat format { ImageFormat::SBGR };
		ImageFormat depth_format { ImageFormat::Depth };
		bool load_colour { false };
		bool keep_colour { true };
		bool load_depth { false };
		bool keep_depth { true };
		bool has_depth { true };
		bool should_present { false };
		std::string debug_name {"UnknownFramebuffer"};
	};

	class Framebuffer {
	public:
		virtual ~Framebuffer() = default;

		virtual void force_recreation() = 0;
		virtual void recreate(bool should_clean) = 0;

		virtual Image& get_image(std::uint32_t index) = 0;
		Image& get_image() { return get_image(0); };

		virtual Disarray::RenderPass& get_render_pass() = 0;

		static Ref<Framebuffer> construct(Disarray::Device&, Disarray::Swapchain&, const FramebufferProperties&);
	};

} // namespace Disarray