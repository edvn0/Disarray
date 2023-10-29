#pragma once

#include <glm/glm.hpp>

#include <fmt/core.h>

#include <functional>
#include <span>
#include <tuple>

#include "Forward.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/IndexBuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/RenderCommandQueue.hpp"
#include "graphics/RendererProperties.hpp"
#include "graphics/TextRenderer.hpp"
#include "graphics/VertexBuffer.hpp"

using VkDescriptorSet = struct VkDescriptorSet_T*;
using VkDescriptorSetLayout = struct VkDescriptorSetLayout_T*;

namespace Disarray {

struct RendererProperties {
	std::string debug_name { "Unknown" };
};

using DescriptorSet = std::uint32_t;
using DescriptorBinding = std::uint32_t;

enum class UBOIdentifier : std::uint8_t {
	Default,
	Camera,
	PointLight,
	ShadowPass,
	DirectionalLight,
	Glyph,
	ImageIndices,
};

enum class RenderPasses : std::uint8_t {
	Text,
	PlanarGeometry,
};

class IGraphicsResource {
public:
	virtual ~IGraphicsResource() = default;

	virtual auto recreate(bool should_clean, const Extent& extent) -> void = 0;

	virtual auto get_pipeline_cache() -> PipelineCache& = 0;
	virtual auto get_texture_cache() -> TextureCache& = 0;
	[[nodiscard]] virtual auto get_pipeline_cache() const -> const PipelineCache& = 0;
	[[nodiscard]] virtual auto get_texture_cache() const -> const TextureCache& = 0;

	virtual void expose_to_shaders(std::span<const Ref<Disarray::Texture>> images, DescriptorSet set, DescriptorBinding binding) = 0;
	virtual void expose_to_shaders(const Disarray::StorageBuffer& buffer, DescriptorSet set, DescriptorBinding binding) = 0;
	virtual void expose_to_shaders(std::span<const Disarray::Texture*> images, DescriptorSet set, DescriptorBinding binding) = 0;
	virtual void expose_to_shaders(const Disarray::Image& images, DescriptorSet set, DescriptorBinding binding) = 0;
	virtual void expose_to_shaders(const Disarray::Texture& images, DescriptorSet set, DescriptorBinding binding) = 0;
	[[nodiscard]] virtual auto get_descriptor_set(DescriptorSet, DescriptorBinding) const -> VkDescriptorSet = 0;
	[[nodiscard]] virtual auto get_descriptor_set(DescriptorSet) const -> VkDescriptorSet = 0;
	[[nodiscard]] virtual auto get_descriptor_set() const -> VkDescriptorSet = 0;
	[[nodiscard]] virtual auto get_descriptor_set_layouts() const -> const std::vector<VkDescriptorSetLayout>& = 0;

	[[nodiscard]] virtual auto get_push_constant() const -> const PushConstant* = 0;
	virtual auto get_editable_push_constant() -> PushConstant& = 0;

	virtual auto get_editable_ubos() -> std::tuple<UBO&, CameraUBO&, PointLights&, ShadowPassUBO&, DirectionalLightUBO&, GlyphUBO&> = 0;

	virtual void update_ubo() = 0;
	virtual void update_ubo(std::size_t ubo_index) = 0;
	virtual void update_ubo(UBOIdentifier identifier) = 0;
};

class Renderer : public ReferenceCountable {
public:
	virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&, bool explicit_clear) = 0;
	virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&, bool explicit_clear, const glm::vec2& mouse_position) = 0;
	virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&) = 0;
	virtual void begin_pass(Disarray::CommandExecutor&) = 0;
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
	virtual void fullscreen_quad_pass(Disarray::CommandExecutor&, const Extent& extent) = 0;

	virtual void on_resize() = 0;
	virtual void clear_pass(Disarray::CommandExecutor&, RenderPasses passes) = 0;

	template <std::size_t T> void clear_pass(Disarray::CommandExecutor& executor, const std::array<RenderPasses, T>& passes)
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

	virtual void bind_pipeline(Disarray::CommandExecutor&, const Disarray::Pipeline&, PipelineBindPoint = PipelineBindPoint::BindPointGraphics) = 0;
	virtual void bind_descriptor_sets(Disarray::CommandExecutor&, const Disarray::Pipeline&) = 0;

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

	virtual void draw_mesh_instanced(
		Disarray::CommandExecutor&, std::size_t count, const Disarray::VertexBuffer&, const Disarray::IndexBuffer&, const Disarray::Pipeline&)
		= 0;

	virtual void draw_aabb(Disarray::CommandExecutor&, const Disarray::AABB&, const glm::vec4&, const glm::mat4& transform) = 0;

	virtual void draw_identifier(Disarray::CommandExecutor&, const Disarray::Pipeline&, std::uint32_t identifier, const glm::mat4& transform) = 0;

	virtual void draw_text(std::string_view text, const glm::uvec2& position, float size, const glm::vec4& colour) = 0;
	virtual void draw_text(std::string_view text, const glm::vec3& position, float size, const glm::vec4& colour) = 0;
	virtual void draw_text(std::string_view text, const glm::mat4& transform, float size, const glm::vec4& colour) = 0;
	virtual void draw_text(std::string_view text, const glm::uvec2& position, const glm::vec4& colour)
	{
		return draw_text(text, position, 1.0F, colour);
	};
	virtual void draw_text(std::string_view text, const glm::uvec2& position, float size) { draw_text(text, position, size, { 1, 1, 1, 1 }); };
	virtual void draw_text(std::string_view text, const glm::uvec2& position) { return draw_text(text, position, 1.0F, { 1, 1, 1, 1 }); };
	virtual void draw_text(std::string_view text, const glm::vec3& position, float size) { draw_text(text, position, size, { 1, 1, 1, 1 }); };

	template <typename... Args> void draw_text(const glm::uvec2& position, fmt::format_string<Args...> fmt_string, Args&&... args)
	{
		return draw_text(fmt::format(fmt_string, std::forward<Args>(args)...), position, 1.0F);
	};
	template <typename... Args> void draw_text(const glm::vec3& position, fmt::format_string<Args...> fmt_string, Args&&... args)
	{
		return draw_text(fmt::format(fmt_string, std::forward<Args>(args)...), position, 1.0F);
	};
	template <typename... Args>
	void draw_text(const glm::uvec2& position, const glm::vec4& colour, fmt::format_string<Args...> fmt_string, Args&&... args)
	{
		return draw_text(fmt::format(fmt_string, std::forward<Args>(args)...), position, 1.0F, colour);
	};
	template <typename... Args>
	void draw_text(const glm::vec3& position, const glm::vec4& colour, fmt::format_string<Args...> fmt_string, Args&&... args)
	{
		return draw_text(fmt::format(fmt_string, std::forward<Args>(args)...), position, 1.0F, colour);
	};

	virtual void submit_batched_geometry(Disarray::CommandExecutor&) = 0;
	virtual void on_batch_full(std::function<void(Renderer&)>&&) = 0;
	virtual void flush_batch(Disarray::CommandExecutor&) = 0;

	virtual void begin_frame(const Camera& camera) = 0;
	virtual void begin_frame(const glm::mat4& view, const glm::mat4& proj, const glm::mat4& view_projection) = 0;
	virtual void end_frame() = 0;

	virtual void force_recreation() = 0;
	virtual auto get_pipeline_cache() -> PipelineCache& = 0;
	virtual auto get_texture_cache() -> TextureCache& = 0;

	virtual auto get_text_renderer() -> TextRenderer& = 0;
	virtual auto get_batch_renderer() -> BatchRenderer& = 0;

	auto get_graphics_resource() -> IGraphicsResource& { return *graphics_resource; }
	[[nodiscard]] virtual auto get_composite_pass_image() const -> const Disarray::Image& = 0;

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
