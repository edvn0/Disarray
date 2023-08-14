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

enum class FramebufferBlendMode { None = 0, OneZero, SrcAlphaOneMinusSrcAlpha, Additive, Zero_SrcColor };

struct FramebufferTextureSpecification {
	explicit(false) FramebufferTextureSpecification(ImageFormat fmt, bool should_blend = true)
		: format(fmt)
		, blend(should_blend)
	{
	}

	ImageFormat format;
	bool blend { true };
	FramebufferBlendMode blend_mode { FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha };
};

struct FramebufferAttachmentSpecification {
	explicit(false) FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification>& attachments)
		: texture_attachments(attachments)
	{
	}

	std::vector<FramebufferTextureSpecification> texture_attachments {};
};

struct FramebufferProperties {
	Extent extent { 0, 0 };
	FramebufferAttachmentSpecification attachments {};
	glm::vec4 clear_colour { 0.0f, 0.0f, 0.0f, 1.0f };
	float depth_clear_value { 0.0f };
	bool clear_colour_on_load { true };
	bool clear_depth_on_load { true };
	bool should_blend { true };
	FramebufferBlendMode blend_mode { FramebufferBlendMode::None };
	bool should_present { false };
	SampleCount samples { SampleCount::One };
	std::string debug_name { "UnknownFramebuffer" };
};

class Framebuffer : public ReferenceCountable {
	DISARRAY_OBJECT(Framebuffer)
public:
	virtual Disarray::Image& get_image(std::uint32_t index) = 0;
	virtual Disarray::Image& get_depth_image() = 0;
	Disarray::Image& get_image() { return get_image(0); };

	virtual Disarray::RenderPass& get_render_pass() = 0;

	virtual std::uint32_t get_colour_attachment_count() const = 0;
	virtual FramebufferProperties& get_properties() = 0;
	virtual const FramebufferProperties& get_properties() const = 0;
	virtual bool has_depth() = 0;

	static Ref<Framebuffer> construct(const Disarray::Device&, const FramebufferProperties&);
};

} // namespace Disarray
