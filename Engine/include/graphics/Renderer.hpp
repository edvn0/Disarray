#pragma once

#include <glm/glm.hpp>

#include <functional>
#include <span>
#include <tuple>

#include "Forward.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Pipeline.hpp"
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

	virtual auto recreate(bool should_clean, const Extent& extent) -> void = 0;

	virtual auto get_pipeline_cache() -> PipelineCache& = 0;
	virtual auto get_texture_cache() -> TextureCache& = 0;

	virtual void expose_to_shaders(std::span<const Ref<Texture>>) = 0;
	virtual void expose_to_shaders(const Image&) = 0;
	virtual void expose_to_shaders(const Texture&) = 0;
	[[nodiscard]] virtual auto get_descriptor_set(std::uint32_t, std::uint32_t) const -> VkDescriptorSet = 0;
	[[nodiscard]] virtual auto get_descriptor_set() const -> VkDescriptorSet = 0;
	[[nodiscard]] virtual auto get_descriptor_set_layouts() const -> const std::vector<VkDescriptorSetLayout>& = 0;

	[[nodiscard]] virtual auto get_push_constant() const -> const PushConstant* = 0;
	virtual auto get_editable_push_constant() -> PushConstant& = 0;

	virtual auto get_editable_ubos() -> std::tuple<UBO&, CameraUBO&, PointLights&, ShadowPassUBO&, DirectionalLightUBO&> = 0;

	virtual void update_ubo() = 0;
};

class Renderer : public ReferenceCountable {
public:
	virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&, bool explicit_clear) = 0;
	virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&) = 0;
	virtual void begin_pass(Disarray::CommandExecutor&) = 0;
	virtual void end_pass(Disarray::CommandExecutor&) = 0;

	virtual void on_resize() = 0;

	virtual void bind_pipeline(Disarray::CommandExecutor&, const Disarray::Pipeline&, PipelineBindPoint = PipelineBindPoint::BindPointGraphics) = 0;

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

	virtual void draw_submeshes(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const Disarray::Texture&,
		const glm::vec4& colour, const glm::mat4& transform = glm::identity<glm::mat4>(), const std::uint32_t identifier = 0)
		= 0;

	virtual void draw_text(std::string_view text, const glm::uvec2& position, float size) = 0;

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

	static auto construct(const Disarray::Device&, const Disarray::Swapchain&, const RendererProperties&) -> Ref<Disarray::Renderer>;
	static auto construct_unique(const Disarray::Device&, const Disarray::Swapchain&, const RendererProperties&) -> Scope<Disarray::Renderer>;

protected:
	Renderer(Scope<IGraphicsResource> resource)
		: graphics_resource { std::move(resource) }
	{
	}

private:
	Scope<IGraphicsResource> graphics_resource { nullptr };
};

} // namespace Disarray
