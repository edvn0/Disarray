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
#include "graphics/RendererProperties.hpp"
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
struct VkDescriptorSetAllocateInfo;

namespace Disarray::Vulkan {

class Renderer : public Disarray::Renderer {
	DISARRAY_MAKE_NONCOPYABLE(Renderer)
	using BaseRenderer = Disarray::Renderer;

public:
	Renderer(const Disarray::Device&, const Disarray::Swapchain&, const RendererProperties&);
	~Renderer() override;

	void construct_sub_renderers(const Disarray::Device&, App& app) override;

	void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&, bool explicit_clear, const RenderAreaExtent&) override;
	void begin_pass(
		Disarray::CommandExecutor&, const Disarray::Framebuffer&, bool explicit_clear, const RenderAreaExtent& render_area_extent) override;
	void end_pass(Disarray::CommandExecutor&) override;

	void text_rendering_pass(Disarray::CommandExecutor&) override;
	void planar_geometry_pass(Disarray::CommandExecutor&) override;
	void fullscreen_quad_pass(Disarray::CommandExecutor&, const Disarray::Pipeline&) override;

	// IGraphics
	void draw_mesh_instanced(Disarray::CommandExecutor&, std::size_t count, const Disarray::VertexBuffer&, const Disarray::IndexBuffer&,
		const Disarray::Pipeline&) override;

	void draw_mesh(Disarray::CommandExecutor&, const Disarray::VertexBuffer&, const Disarray::IndexBuffer&, const Disarray::Pipeline&,
		const Disarray::Material&, const TransformMatrix&, const ColourVector&) override;
	void draw_mesh(Disarray::CommandExecutor&, const Disarray::VertexBuffer&, const Disarray::IndexBuffer&, const Disarray::Pipeline&,
		const TransformMatrix&, const ColourVector&) override;
	void draw_mesh(
		Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const TransformMatrix&, const ColourVector&) override;
	void draw_mesh(
		Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const Disarray::Material&, const TransformMatrix&, const ColourVector&) override;
	void draw_mesh_without_bind(
		Disarray::CommandExecutor&, const Disarray::Mesh&) override;

	void draw_billboarded_text(std::string_view, const TransformMatrix&, float, const ColourVector&) override;
	void draw_text(std::string_view, const glm::uvec2&, float, const ColourVector&) override;
	void draw_text(std::string_view, const glm::vec3&, float, const ColourVector&) override;
	void draw_text(std::string_view, const TransformMatrix&, float, const ColourVector&) override;
	void draw_planar_geometry(Disarray::Geometry, const Disarray::GeometryProperties&) override;
	void submit_batched_geometry(Disarray::CommandExecutor&) override;
	void on_batch_full(std::function<void(Disarray::Renderer&)>&& func) override { on_batch_full_func = std::move(func); }
	void flush_batch(Disarray::CommandExecutor&) override;
	// End IGraphics

	void on_resize() override;
	auto get_pipeline_cache() -> PipelineCache& override { return get_graphics_resource().get_pipeline_cache(); }
	auto get_texture_cache() -> TextureCache& override { return get_graphics_resource().get_texture_cache(); }

	auto get_batch_renderer() -> BatchRenderer& override { return batch_renderer; }
	auto get_text_renderer() -> TextRenderer& override { return text_renderer; }

	void begin_frame() override;
	void end_frame() override;

	void force_recreation() override;
	void clear_pass(Disarray::CommandExecutor&, RenderPasses) override;
	void clear_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&) override;

	void bind_pipeline(Disarray::CommandExecutor&, const Disarray::Pipeline&, PipelineBindPoint) override;
	void bind_descriptor_sets(Disarray::CommandExecutor&, const Disarray::Pipeline&) override;
	void push_constant(Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline, const void* data, std::size_t size) override;
	void push_constant(Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline) override;

	void set_scissors(Disarray::CommandExecutor&, const glm::vec2& scissor_extent, const glm::vec2& offset) override;
	void set_viewport(Disarray::CommandExecutor&, const glm::vec2& viewport_extent) override;

private:
	void add_geometry_to_batch(Geometry, const GeometryProperties&);
	void draw_billboard_quad(Disarray::CommandExecutor&, const Disarray::Pipeline&);
	void bind_descriptor_sets(Disarray::CommandExecutor&, const Disarray::Pipeline&, const std::span<const VkDescriptorSet>&);

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
