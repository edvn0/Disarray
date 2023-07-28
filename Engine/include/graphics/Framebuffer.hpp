#pragma once

#include "core/Types.hpp"

namespace Disarray {

	class Device;
	class Swapchain;
	class RenderPass;

	struct FramebufferProperties {};

	class Framebuffer {
	public:
		virtual ~Framebuffer() = default;

		virtual void force_recreation() = 0;

		static Ref<Framebuffer> construct(Ref<Device>, Ref<Swapchain>, Ref<RenderPass>, const FramebufferProperties&);
	};

}