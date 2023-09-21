#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <array>

#include "graphics/CommandExecutor.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/RenderBatch.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/Texture.hpp"
#include "graphics/TextureCache.hpp"
#include "graphics/UniformBuffer.hpp"
#include "graphics/VertexBuffer.hpp"
#include "graphics/VertexTypes.hpp"

namespace Disarray::Vulkan {

class Renderer : public Disarray::Renderer {
	DISARRAY_MAKE_NONCOPYABLE(Renderer)
public:
	Renderer(const Disarray::Device&, const Disarray::Swapchain&, const RendererProperties&);
	~Renderer() override;

	void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&, bool explicit_clear) override;
	void begin_pass(Disarray::CommandExecutor& executor, Disarray::Framebuffer& fb) override { begin_pass(executor, fb, false); }
	void begin_pass(Disarray::CommandExecutor& command_executor) override { begin_pass(command_executor, *geometry_framebuffer); }
	void end_pass(Disarray::CommandExecutor&) override;

	// IGraphics
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const GeometryProperties& = {}) override;
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const glm::mat4&) override;
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const glm::mat4&) override;
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const glm::mat4&, const std::uint32_t) override;
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const Disarray::Texture&, const glm::mat4&,
		const std::uint32_t) override;
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const Disarray::Texture&, const glm::vec4&,
		const glm::mat4&, const std::uint32_t) override;

	void draw_submeshes(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const Disarray::Texture&, const glm::vec4&,
		const glm::mat4&, const std::uint32_t) override;
	void draw_planar_geometry(Disarray::Geometry, const Disarray::GeometryProperties&) override;
	void submit_batched_geometry(Disarray::CommandExecutor&) override;
	void on_batch_full(std::function<void(Disarray::Renderer&)>&& func) override { on_batch_full_func = func; }
	void flush_batch(Disarray::CommandExecutor&) override;
	// End IGraphics

	void on_resize() override;
	auto get_pipeline_cache() -> PipelineCache& override { return get_graphics_resource().get_pipeline_cache(); }
	auto get_texture_cache() -> TextureCache& override { return get_graphics_resource().get_texture_cache(); }

	void begin_frame(const Camera&) override;
	void begin_frame(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& view_projection) override;
	void end_frame() override;

	void force_recreation() override;

	void bind_pipeline(Disarray::CommandExecutor&, const Disarray::Pipeline&, PipelineBindPoint = PipelineBindPoint::BindPointGraphics) override;

private:
	void add_geometry_to_batch(Geometry, const GeometryProperties&);

	void draw_submesh(Disarray::CommandExecutor&, const Disarray::VertexBuffer&, const Disarray::IndexBuffer&, const Disarray::Pipeline&,
		const Disarray::Texture&, const glm::vec4&, const glm::mat4&, const std::uint32_t);

	const Disarray::Device& device;
	const Disarray::Swapchain& swapchain;

	BatchRenderer batch_renderer;

	Ref<Disarray::Framebuffer> geometry_framebuffer;
	Ref<Disarray::Framebuffer> quad_framebuffer;

	void bind_descriptor_sets(Disarray::CommandExecutor& executor, VkPipelineLayout pipeline_layout, const std::array<VkDescriptorSet, 3>& desc);

	mutable std::array<VkDescriptorSet, 3> bound { nullptr };
	mutable const Disarray::Pipeline* bound_pipeline { nullptr };

	std::function<void(Disarray::Renderer&)> on_batch_full_func = [](auto&) {};

	RendererProperties props;
	Extent extent;
};

} // namespace Disarray::Vulkan
