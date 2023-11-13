#pragma once

#include <glm/glm.hpp>

#include <array>
#include <span>

#include "graphics/CommandExecutor.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/RenderBatch.hpp"
#include "graphics/Renderer.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/TextRenderer.hpp"
#include "graphics/Texture.hpp"
#include "graphics/TextureCache.hpp"
#include "graphics/UniformBuffer.hpp"
#include "graphics/VertexBuffer.hpp"
#include "graphics/VertexTypes.hpp"

using VkPipelineLayout = struct VkPipelineLayout_T*;
using VkDevice = struct VkDevice_T*;
using VkDescriptorSet = struct VkDescriptorSet_T*;

namespace Disarray::Vulkan {

class Renderer : public Disarray::Renderer {
	DISARRAY_MAKE_NONCOPYABLE(Renderer)
	using BaseRenderer = Disarray::Renderer;

public:
	Renderer(const Disarray::Device&, const Disarray::Swapchain&, const RendererProperties&);
	~Renderer() override;

	void construct_sub_renderers(const Disarray::Device&, App& app) override;

	void begin_pass(Disarray::CommandExecutor& executor, Disarray::Framebuffer& framebuffer, bool explicit_clear,
		const RenderAreaExtent& render_area_extent) override;
	void begin_pass(Disarray::CommandExecutor& executor, const Disarray::Framebuffer& framebuffer, bool explicit_clear,
		const RenderAreaExtent& render_area_extent) override;
	void end_pass(Disarray::CommandExecutor&) override;

	void text_rendering_pass(Disarray::CommandExecutor& /*unused*/) override;
	void planar_geometry_pass(Disarray::CommandExecutor& /*unused*/) override;
	void fullscreen_quad_pass(Disarray::CommandExecutor& executor, const Disarray::Pipeline& fullscreen_pipeline) override;

	// IGraphics
	void draw_mesh_instanced(Disarray::CommandExecutor&, std::size_t count, const Disarray::VertexBuffer&, const Disarray::IndexBuffer&,
		const Disarray::Pipeline&) override;

	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const GeometryProperties& = {}) override;
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const glm::mat4&) override;
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const glm::mat4&) override;
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const glm::mat4&, const std::uint32_t) override;
	void draw_mesh(Disarray::CommandExecutor& executor, const Disarray::Mesh& mesh, const Disarray::Pipeline& mesh_pipeline, const glm::vec4& colour,
		const glm::mat4& transform) override;
	void draw_mesh(Disarray::CommandExecutor& executor, const Disarray::VertexBuffer& vertices, const Disarray::IndexBuffer& indices,
		const Disarray::Pipeline& mesh_pipeline, const glm::vec4& colour, const glm::mat4& transform) override;
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const Disarray::Texture&, const glm::mat4&,
		const std::uint32_t) override;
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const Disarray::Texture&, const glm::vec4&,
		const glm::mat4&, const std::uint32_t) override;

	void draw_billboarded_text(std::string_view text, const glm::mat4& transform, float size, const glm::vec4& colour) override;
	void draw_text(std::string_view text, const glm::uvec2& position, float size, const glm::vec4& colour) override;
	void draw_text(std::string_view text, const glm::vec3& position, float size, const glm::vec4& colour) override;
	void draw_text(std::string_view text, const glm::mat4& transform, float size, const glm::vec4& colour) override;
	void draw_planar_geometry(Disarray::Geometry /*unused*/, const Disarray::GeometryProperties& /*unused*/) override;
	void submit_batched_geometry(Disarray::CommandExecutor& /*unused*/) override;
	void on_batch_full(std::function<void(Disarray::Renderer&)>&& func) override { on_batch_full_func = std::move(func); }
	void flush_batch(Disarray::CommandExecutor& /*unused*/) override;
	// End IGraphics

	void on_resize() override;
	auto get_pipeline_cache() -> PipelineCache& override { return get_graphics_resource().get_pipeline_cache(); }
	auto get_texture_cache() -> TextureCache& override { return get_graphics_resource().get_texture_cache(); }

	auto get_batch_renderer() -> BatchRenderer& override { return batch_renderer; }
	auto get_text_renderer() -> TextRenderer& override { return text_renderer; }

	void begin_frame() override;
	void end_frame() override;

	void force_recreation() override;
	void clear_pass(Disarray::CommandExecutor&, RenderPasses passes) override;
	void clear_pass(Disarray::CommandExecutor&, Disarray::Framebuffer& passes) override;

	void bind_pipeline(Disarray::CommandExecutor&, const Disarray::Pipeline&, PipelineBindPoint = PipelineBindPoint::BindPointGraphics) override;
	void bind_descriptor_sets(Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline) override;

	void set_scissors(Disarray::CommandExecutor& executor, const glm::vec2& scissor_extent, const glm::vec2& offset) override;
	void set_viewport(Disarray::CommandExecutor& executor, const glm::vec2& viewport_extent) override;

private:
	void add_geometry_to_batch(Geometry, const GeometryProperties&);
	void draw_billboard_quad(Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline);
	void bind_descriptor_sets(Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline, const std::span<const VkDescriptorSet>& span);

	const Disarray::Device& device;
	const Disarray::Swapchain& swapchain;

	BatchRenderer batch_renderer;
	TextRenderer text_renderer;

	mutable const Disarray::Pipeline* bound_pipeline { nullptr };
	mutable std::size_t bound_descriptor_set_hash { 0 };
	std::function<void(Disarray::Renderer&)> on_batch_full_func = [](auto&) {};

	RendererProperties props;
	Extent extent;
};

} // namespace Disarray::Vulkan
