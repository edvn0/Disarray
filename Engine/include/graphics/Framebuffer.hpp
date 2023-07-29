#pragma once

#include "Forward.hpp"

namespace Disarray {

	struct FramebufferProperties {
		bool has_depth {true};
	};

	class Framebuffer {
	public:
		virtual ~Framebuffer() = default;

		virtual void force_recreation() = 0;

		static Ref<Framebuffer> construct(Ref<Device>, Ref<Swapchain>, Ref<Disarray::PhysicalDevice> physical_device, Ref<RenderPass>, const FramebufferProperties&);
	};

}