#pragma once

#include <glm/glm.hpp>

#include <functional>

#include "Forward.hpp"
#include "graphics/RendererProperties.hpp"

using VkDescriptorSet = struct VkDescriptorSet_T*;
using VkDescriptorSetLayout = struct VkDescriptorSetLayout_T*;

namespace Disarray {

struct RendererProperties {
	std::string debug_name { "Unknown" };
};

class IGraphics {
public:
	virtual ~IGraphics() = default;

	virtual void draw_planar_geometry(Geometry, const GeometryProperties&) = 0;
	virtual void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const GeometryProperties&) = 0;
	virtual void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const glm::mat4& transform = glm::identity<glm::mat4>()) = 0;
	virtual void draw_mesh(
		Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const glm::mat4& transform = glm::identity<glm::mat4>())
		= 0;
	virtual void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&,
		const glm::mat4& transform = glm::identity<glm::mat4>(), const std::uint32_t identifier = 0)
		= 0;
	virtual void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const Disarray::Texture&,
		const glm::mat4& transform = glm::identity<glm::mat4>(), const std::uint32_t identifier = 0)
		= 0;
	virtual void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const Disarray::Texture&,
		const glm::vec4& colour, const glm::mat4& transform = glm::identity<glm::mat4>(), const std::uint32_t identifier = 0)
		= 0;
	virtual void submit_batched_geometry(Disarray::CommandExecutor&) = 0;
	virtual void on_batch_full(std::function<void(Renderer&)>&&) = 0;
	virtual void flush_batch(Disarray::CommandExecutor&) = 0;
};

class IGraphicsResource {
public:
	virtual ~IGraphicsResource() = default;

	virtual void expose_to_shaders(Image&) = 0;
	virtual void expose_to_shaders(Texture&) = 0;
	virtual VkDescriptorSet get_descriptor_set(std::uint32_t, std::uint32_t) = 0;
	virtual VkDescriptorSet get_descriptor_set() = 0;
	virtual const std::vector<VkDescriptorSetLayout>& get_descriptor_set_layouts() = 0;

	virtual const PushConstant* get_push_constant() const = 0;
	virtual PushConstant& get_editable_push_constant() = 0;

	virtual const UBO* get_ubo() const = 0;
	virtual UBO& get_editable_ubo() = 0;
};

class Renderer : public IGraphics, public IGraphicsResource, public ReferenceCountable {
public:
	virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&, bool explicit_clear) = 0;
	virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&) = 0;
	virtual void begin_pass(Disarray::CommandExecutor&) = 0;
	virtual void end_pass(Disarray::CommandExecutor&) = 0;

	virtual void on_resize() = 0;

	virtual PipelineCache& get_pipeline_cache() = 0;
	virtual TextureCache& get_texture_cache() = 0;

	virtual void begin_frame(Camera& camera) = 0;
	virtual void end_frame() = 0;

	virtual void force_recreation() = 0;

	static Ref<Renderer> construct(const Disarray::Device&, Disarray::Swapchain&, const RendererProperties&);
};

} // namespace Disarray
