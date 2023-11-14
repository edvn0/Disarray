#pragma once

#include "Forward.hpp"

#include <glm/glm.hpp>

#include <fmt/core.h>

#include <functional>
#include <span>
#include <tuple>

#include "core/Types.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/RenderCommandQueue.hpp"
#include "graphics/RendererProperties.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/TextRenderer.hpp"
#include "graphics/UniformBufferSet.hpp"
#include "graphics/VertexBuffer.hpp"

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
	[[nodiscard]] virtual auto get_pipeline_cache() const -> const PipelineCache& = 0;
	[[nodiscard]] virtual auto get_texture_cache() const -> const TextureCache& = 0;

	virtual void expose_to_shaders(const Disarray::StorageBuffer& buffer, DescriptorSet set, DescriptorBinding binding) = 0;

	template <class Buffer> void expose_to_shaders(const Disarray::UniformBufferSet<Buffer>& buffer_set, DescriptorSet set, DescriptorBinding binding)
	{
		for (const Scope<Disarray::UniformBuffer>& buffer : buffer_set) {
			expose_to_shaders(*buffer, set, binding);
		}
	};

	virtual void expose_to_shaders(const Disarray::UniformBuffer& buffer, DescriptorSet set, DescriptorBinding binding) = 0;
	virtual void expose_to_shaders(std::span<const Ref<Disarray::Texture>> images, DescriptorSet set, DescriptorBinding binding) = 0;
	virtual void expose_to_shaders(std::span<const Disarray::Texture*> images, DescriptorSet set, DescriptorBinding binding) = 0;
	virtual void expose_to_shaders(const Disarray::Image& images, DescriptorSet set, DescriptorBinding binding) = 0;
	virtual void expose_to_shaders(const Disarray::Texture& images, DescriptorSet set, DescriptorBinding binding) = 0;

	[[nodiscard]] virtual auto get_descriptor_set(FrameIndex, DescriptorSet) const -> VkDescriptorSet = 0;
	[[nodiscard]] virtual auto get_descriptor_set(DescriptorSet) const -> VkDescriptorSet = 0;
	[[nodiscard]] virtual auto get_descriptor_set() const -> VkDescriptorSet = 0;
	[[nodiscard]] virtual auto get_descriptor_set_layouts() const -> const std::vector<VkDescriptorSetLayout>& = 0;

	virtual void push_constant(Disarray::CommandExecutor&, const Disarray::Pipeline&) = 0;
	virtual void push_constant(Disarray::CommandExecutor&, const Disarray::Pipeline&, const void* data, std::size_t size) = 0;

	[[nodiscard]] virtual auto get_push_constant() const -> const PushConstant* = 0;
	virtual auto get_editable_push_constant() -> PushConstant& = 0;

	[[nodiscard]] virtual auto get_image_count() const -> std::uint32_t = 0;
};

class Renderer : public ReferenceCountable {
public:
	virtual void construct_sub_renderers(const Disarray::Device&, Disarray::App&) = 0;

	virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&, bool explicit_clear, const RenderAreaExtent&) = 0;
	virtual void begin_pass(Disarray::CommandExecutor&, const Disarray::Framebuffer&, bool explicit_clear, const RenderAreaExtent&) = 0;
	virtual void begin_pass(Disarray::CommandExecutor& executor, Disarray::Framebuffer& framebuffer, bool explicit_clear)
	{
		begin_pass(executor, framebuffer, explicit_clear, RenderAreaExtent { framebuffer });
	}
	virtual void begin_pass(Disarray::CommandExecutor& executor, Disarray::Framebuffer& framebuffer)
	{
		begin_pass(executor, framebuffer, false, RenderAreaExtent { framebuffer });
	}
	virtual void end_pass(Disarray::CommandExecutor&) = 0;

	/**
	 * @brief This is an external pass, i.e. requires that the underlying implementation provides a render pass.
	 */
	virtual void text_rendering_pass(Disarray::CommandExecutor&) = 0;

	/**
	 * @brief This is currently an intrinsic pass, i.e. requires a started render pass.
	 */
	virtual void planar_geometry_pass(Disarray::CommandExecutor&) = 0;

	/**
	 * @brief This is an external pass, i.e. requires that the underlying implementation provides a render pass.
	 */
	virtual void fullscreen_quad_pass(Disarray::CommandExecutor&, const Disarray::Pipeline& fullscreen_pipeline) = 0;

	virtual void on_resize() = 0;
	virtual void clear_pass(Disarray::CommandExecutor&, RenderPasses passes) = 0;

	template <std::size_t T> void clear_pass(Disarray::CommandExecutor& executor, std::array<RenderPasses, T>&& passes)
	{
		for (const auto& pass : passes) {
			clear_pass(executor, pass);
		}
	}

	template <RenderPasses... Passes> void clear_pass(Disarray::CommandExecutor& executor)
	{
		constexpr auto pass_count = sizeof...(Passes);
		clear_pass<pass_count>(executor, std::array<RenderPasses, pass_count> { Passes... });
	}

	virtual void clear_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&) = 0;

	virtual void bind_pipeline(Disarray::CommandExecutor&, const Disarray::Pipeline&, PipelineBindPoint) = 0;
	void bind_pipeline(Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline)
	{
		bind_pipeline(executor, pipeline, PipelineBindPoint::BindPointGraphics);
	};
	virtual void bind_descriptor_sets(Disarray::CommandExecutor&, const Disarray::Pipeline&) = 0;

	virtual void push_constant(Disarray::CommandExecutor&, const Disarray::Pipeline&, const void* data, std::size_t size) = 0;
	virtual void push_constant(Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline) = 0;

	template <class T>
		requires(sizeof(T) <= 128)
	void push_constant(Disarray::CommandExecutor& executor, const Disarray::Pipeline& pipeline, const T& data = {})
	{
		push_constant(executor, pipeline, &data, sizeof(T));
	}

	virtual void draw_mesh(CommandExecutor&, const VertexBuffer&, const IndexBuffer&, const Pipeline&, const TransformMatrix&, const ColourVector&)
		= 0;
	virtual void draw_mesh(
		CommandExecutor&, const VertexBuffer&, const IndexBuffer&, const Pipeline&, const Material&, const TransformMatrix&, const ColourVector&)
		= 0;
	virtual void draw_mesh(CommandExecutor&, const Mesh&, const Pipeline&, const TransformMatrix&, const ColourVector&) = 0;
	virtual void draw_mesh(CommandExecutor&, const Mesh&, const Pipeline&, const Material&, const TransformMatrix&, const ColourVector&) = 0;
	virtual void draw_mesh_instanced(
		CommandExecutor&, std::size_t count, const Disarray::VertexBuffer&, const Disarray::IndexBuffer&, const Disarray::Pipeline&)
		= 0;
	/**
		note: Intended usage is for submeshes, where the pipeline and descriptor sets will be constants (for now!)
	*/
	virtual void draw_mesh_without_bind(
		CommandExecutor&, const Disarray::Mesh&)
		= 0;
	virtual void draw_planar_geometry(Disarray::Geometry, const GeometryProperties&) = 0;

	virtual void draw_text(std::string_view text, const glm::uvec2& position, float size, const ColourVector&) = 0;
	virtual void draw_text(std::string_view text, const glm::vec3& position, float size, const ColourVector&) = 0;
	virtual void draw_text(std::string_view text, const TransformMatrix&, float size, const ColourVector&) = 0;
	virtual void draw_billboarded_text(std::string_view text, const TransformMatrix&, float size, const ColourVector&) = 0;
	void draw_text(std::string_view text, const glm::uvec2& position, const ColourVector& colour) { return draw_text(text, position, 1.0F, colour); };
	void draw_text(std::string_view text, const glm::uvec2& position, float size) { draw_text(text, position, size, { 1, 1, 1, 1 }); };
	void draw_text(std::string_view text, const glm::uvec2& position) { return draw_text(text, position, 1.0F, { 1, 1, 1, 1 }); };
	void draw_text(std::string_view text, const glm::vec3& position, float size) { draw_text(text, position, size, { 1, 1, 1, 1 }); };

	template <typename... Args> void draw_text(const glm::uvec2& position, fmt::format_string<Args...> fmt_string, Args&&... args)
	{
		return draw_text(fmt::format(fmt_string, std::forward<Args>(args)...), position, 1.0F);
	};
	template <typename... Args> void draw_text(const glm::vec3& position, fmt::format_string<Args...> fmt_string, Args&&... args)
	{
		return draw_text(fmt::format(fmt_string, std::forward<Args>(args)...), position, 1.0F);
	};
	template <typename... Args>
	void draw_text(const glm::uvec2& position, const ColourVector& colour, fmt::format_string<Args...> fmt_string, Args&&... args)
	{
		return draw_text(fmt::format(fmt_string, std::forward<Args>(args)...), position, 1.0F, colour);
	};
	template <typename... Args>
	void draw_text(const glm::vec3& position, const ColourVector& colour, fmt::format_string<Args...> fmt_string, Args&&... args)
	{
		return draw_text(fmt::format(fmt_string, std::forward<Args>(args)...), position, 1.0F, colour);
	};

	virtual void set_scissors(Disarray::CommandExecutor&, const glm::vec2& scissor_extent, const glm::vec2& offset) = 0;
	virtual void set_viewport(Disarray::CommandExecutor&, const glm::vec2& viewport_extent) = 0;

	virtual void submit_batched_geometry(Disarray::CommandExecutor&) = 0;
	virtual void on_batch_full(std::function<void(Renderer&)>&&) = 0;
	virtual void flush_batch(Disarray::CommandExecutor&) = 0;

	virtual void begin_frame() = 0;
	virtual void end_frame() = 0;

	virtual void force_recreation() = 0;
	virtual auto get_pipeline_cache() -> PipelineCache& = 0;
	virtual auto get_texture_cache() -> TextureCache& = 0;

	virtual auto get_text_renderer() -> TextRenderer& = 0;
	virtual auto get_batch_renderer() -> BatchRenderer& = 0;

	auto get_graphics_resource() -> IGraphicsResource& { return *graphics_resource; }

	static auto construct(const Disarray::Device&, const Disarray::Swapchain&, const RendererProperties&) -> Ref<Disarray::Renderer>;
	static auto construct_unique(const Disarray::Device&, const Disarray::Swapchain&, const RendererProperties&) -> Scope<Disarray::Renderer>;

	template <class Func> static auto submit(Func&& func) { get_render_command_queue().allocate(std::forward<Func>(func)); }

	static auto execute_queue() { get_render_command_queue().execute(); }

	static auto get_render_command_queue() -> RenderCommandQueue& { return command_queue; }

protected:
	explicit Renderer(Scope<IGraphicsResource> resource)
		: graphics_resource { std::move(resource) }
	{
	}

private:
	Scope<IGraphicsResource> graphics_resource { nullptr };

	static inline RenderCommandQueue command_queue { {} };
};

} // namespace Disarray
