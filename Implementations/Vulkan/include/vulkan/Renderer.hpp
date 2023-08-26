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

static constexpr auto max_batch_renderer_objects = 1000;

// TODO: Make this dynamic
static constexpr auto set_count = 2;

class Renderer : public Disarray::Renderer {
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
	void draw_planar_geometry(Disarray::Geometry, const Disarray::GeometryProperties&) override;
	void submit_batched_geometry(Disarray::CommandExecutor&) override;
	void on_batch_full(std::function<void(Disarray::Renderer&)>&& func) override { on_batch_full_func = func; }
	void flush_batch(Disarray::CommandExecutor&) override;
	// End IGraphics

	// IGraphicsResource
	void expose_to_shaders(Disarray::Image&) override;
	void expose_to_shaders(Disarray::Texture& tex) override { expose_to_shaders(tex.get_image()); };
	VkDescriptorSet get_descriptor_set(std::uint32_t frame_index, std::uint32_t set) override
	{
		return descriptor_sets[(frame_index * set_count) + set];
	}
	VkDescriptorSet get_descriptor_set() override { return get_descriptor_set(swapchain.get_current_frame(), 0); };
	const std::vector<VkDescriptorSetLayout>& get_descriptor_set_layouts() override { return layouts; }
	// End IGraphicsResource

	void on_resize() override;
	PipelineCache& get_pipeline_cache() override { return pipeline_cache; }
	TextureCache& get_texture_cache() override { return texture_cache; }

	void begin_frame(const Camera&) override;
	void begin_frame(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& view_projection) override;
	void end_frame() override;

	void force_recreation() override;

	const PushConstant* get_push_constant() const override { return &pc; }
	PushConstant& get_editable_push_constant() override { return pc; }

	const UBO* get_ubo() const override { return &uniform; }
	UBO& get_editable_ubo() override { return uniform; }

private:
	void add_geometry_to_batch(Geometry, const GeometryProperties&);

	const Disarray::Device& device;
	const Disarray::Swapchain& swapchain;

	Disarray::PipelineCache pipeline_cache;
	Disarray::TextureCache texture_cache;

	BatchRenderer<max_batch_renderer_objects> batch_renderer;

	Ref<Disarray::Framebuffer> geometry_framebuffer;
	Ref<Disarray::Framebuffer> quad_framebuffer;

	std::function<void(Disarray::Renderer&)> on_batch_full_func = [](auto&) {};

	VkDescriptorPool pool;
	// For every frame, we have a vector of descriptor sets

	std::vector<VkDescriptorSet> descriptor_sets;
	std::vector<VkDescriptorSetLayout> layouts;
	void initialise_descriptors();

	UBO uniform {};
	std::vector<Ref<UniformBuffer>> frame_ubos;

	RendererProperties props;
	Extent extent;
	PushConstant pc {};
};

} // namespace Disarray::Vulkan
