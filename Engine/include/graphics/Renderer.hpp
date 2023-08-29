#pragma once

#include <glm/glm.hpp>

#include <functional>
#include <tuple>

#include "Forward.hpp"
#include "graphics/RendererProperties.hpp"

using VkDescriptorSet = struct VkDescriptorSet_T*;
using VkDescriptorSetLayout = struct VkDescriptorSetLayout_T*;

namespace Disarray {

struct RendererProperties {
	std::string debug_name { "Unknown" };
};

class IGraphicsResource {
public:
	virtual ~IGraphicsResource() = default;

	virtual auto get_pipeline_cache() -> PipelineCache& = 0;
	virtual auto get_texture_cache() -> TextureCache& = 0;

	virtual void expose_to_shaders(Image&) = 0;
	virtual void expose_to_shaders(Texture&) = 0;
	[[nodiscard]] virtual VkDescriptorSet get_descriptor_set(std::uint32_t, std::uint32_t) const = 0;
	[[nodiscard]] virtual VkDescriptorSet get_descriptor_set() const = 0;
	virtual const std::vector<VkDescriptorSetLayout>& get_descriptor_set_layouts() const = 0;

	[[nodiscard]] virtual const PushConstant* get_push_constant() const = 0;
	virtual PushConstant& get_editable_push_constant() = 0;

	virtual std::tuple<UBO&, CameraUBO&, PointLights&> get_editable_ubos() = 0;

	virtual void update_ubo() = 0;
};

class Renderer : public ReferenceCountable {
public:
	virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&, bool explicit_clear) = 0;
	virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&) = 0;
	virtual void begin_pass(Disarray::CommandExecutor&) = 0;
	virtual void end_pass(Disarray::CommandExecutor&) = 0;

	virtual void on_resize() = 0;

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

	virtual void begin_frame(const Camera& camera) = 0;
	virtual void begin_frame(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& view_projection) = 0;
	virtual void end_frame() = 0;

	virtual void force_recreation() = 0;
	virtual auto get_pipeline_cache() -> PipelineCache& = 0;
	virtual auto get_texture_cache() -> TextureCache& = 0;

	auto get_graphics_resource() -> IGraphicsResource& { return *graphics_resource; }

	static auto construct(const Disarray::Device&, const Disarray::Swapchain&, const RendererProperties&) -> Ref<Renderer>;
	static auto construct_unique(const Disarray::Device&, const Disarray::Swapchain&, const RendererProperties&) -> Scope<Renderer>;

protected:
	Renderer(Scope<IGraphicsResource> resource)
		: graphics_resource { std::move(resource) }
	{
	}

private:
	Scope<IGraphicsResource> graphics_resource { nullptr };
};

} // namespace Disarray
