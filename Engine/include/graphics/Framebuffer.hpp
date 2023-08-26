#pragma once

#include <glm/glm.hpp>

#include "Forward.hpp"
#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"

namespace Disarray {

enum class FramebufferBlendMode : std::uint8_t { None = 0, OneZero, SrcAlphaOneMinusSrcAlpha, Additive, Zero_SrcColor };

struct FramebufferTextureSpecification {
	ImageFormat format { ImageFormat::SBGR };
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
	glm::vec4 clear_colour { 0.0F, 0.0F, 0.0F, 0.0F };
	float depth_clear_value { 0.0F };
	bool clear_colour_on_load { true };
	bool clear_depth_on_load { true };
	bool should_blend { true };
	FramebufferBlendMode blend_mode { FramebufferBlendMode::None };
	bool should_present { false };
	SampleCount samples { SampleCount::One };
	std::string debug_name { "UnknownFramebuffer" };
};

class Framebuffer : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(Framebuffer, FramebufferProperties)
public:
	auto get_image() -> Disarray::Image& { return get_image(0); };

	virtual auto get_image(std::uint32_t index) -> Disarray::Image& = 0;
	virtual auto get_depth_image() -> Disarray::Image& = 0;

	virtual auto get_render_pass() -> Disarray::RenderPass& = 0;

	virtual auto get_colour_attachment_count() const -> std::uint32_t = 0;
	virtual auto has_depth() -> bool = 0;

	static Ref<Framebuffer> construct(const Disarray::Device&, FramebufferProperties);
};

} // namespace Disarray
