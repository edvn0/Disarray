#pragma once

#include "Forward.hpp"

#include <glm/fwd.hpp>

#include "core/Collections.hpp"
#include "core/PointerDefinition.hpp"
#include "graphics/BufferSet.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/RendererProperties.hpp"
#include "graphics/UniformBufferSet.hpp"

namespace Disarray {

enum class SceneFramebuffer : std::uint8_t {
	Geometry,
	Identity,
	Shadow,
	FullScreen,
};

class SceneRenderer {
public:
	explicit SceneRenderer(const Disarray::Device&);
	auto construct(Disarray::App&) -> void;
	auto destruct() -> void;

	auto recreate(bool should_clean, const Extent& extent) -> void;
	auto interface() -> void;

	/**
	 * ACTUAL DRAWING
	 */
	auto begin_frame(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& view_projection) -> void;
	auto end_frame() -> void;

	auto draw_text(const Components::Transform&, const Components::Text&, const glm::vec4& colour) -> void;

	template <SceneFramebuffer FB> void begin_pass(bool explicit_clear = false) { begin_pass(*get_framebuffer<FB>(), explicit_clear); }
	void end_pass();
	template <RenderPasses... RP> void clear_pass() { clear_pass(std::array<RenderPasses, sizeof...(RP)> { RP... }); };

	auto draw_identifiers(std::size_t count) -> void;
	auto draw_skybox(const Disarray::Mesh& skybox_mesh) -> void;
	auto draw_point_lights(const Disarray::Mesh& point_light_mesh, std::integral auto count, const Disarray::Pipeline& pipeline) -> void
	{
		draw_point_lights(point_light_mesh, static_cast<std::uint32_t>(count), pipeline);
	}
	auto draw_point_lights(const Disarray::Mesh& point_light_mesh, std::integral auto count) -> void
	{
		draw_point_lights(point_light_mesh, static_cast<std::uint32_t>(count), *get_pipeline("PointLight"));
	}
	auto draw_spot_light(const Disarray::Mesh& point_light_mesh, std::integral auto count, const Disarray::Pipeline& pipeline) -> void
	{
		draw_spot_light(point_light_mesh, static_cast<std::uint32_t>(count), pipeline);
	}
	auto draw_spot_light(const Disarray::Mesh& point_light_mesh, std::integral auto count) -> void
	{
		draw_spot_light(point_light_mesh, static_cast<std::uint32_t>(count), *get_pipeline("SpotLight"));
	}
	auto draw_point_lights(const Disarray::Mesh& point_light_mesh, std::uint32_t count, const Disarray::Pipeline& pipeline) -> void;
	auto draw_planar_geometry(Geometry, const GeometryProperties&) -> void;
	auto draw_aabb(const AABB& aabb, const glm::vec4& colour, const glm::mat4& transform) -> void;
	auto draw_static_submeshes(const Collections::ScopedStringMap<Disarray::MeshSubstructure>&, const Disarray::Pipeline&, const glm::mat4& transform,
		const glm::vec4& colour) -> void;
	auto draw_single_static_mesh(const Disarray::Mesh& mesh, const Disarray::Pipeline& pipeline, const glm::mat4& transform, const glm::vec4& colour)
		-> void;
	auto draw_single_static_mesh(const Disarray::VertexBuffer& vertices, const Disarray::IndexBuffer& indices, const Disarray::Pipeline& pipeline,
		const glm::mat4& transform, const glm::vec4& colour) -> void;

	auto draw_static_mesh(Ref<Disarray::StaticMesh>& mesh, const Disarray::Pipeline& pipeline, const glm::mat4& transform, const glm::vec4& colour)
		-> void;
	/**
	 * END ACTUAL DRAWING
	 */

	/**
	 * PASSES
	 */
	auto planar_geometry_pass() -> void;
	auto text_rendering_pass() -> void;
	auto fullscreen_quad_pass() -> void;
	/**
	 * END PASSES
	 */

	auto get_command_executor() -> CommandExecutor&;
	auto get_pipeline_cache() -> PipelineCache&;
	auto get_texture_cache() -> TextureCache&;
	auto get_graphics_resource() -> IGraphicsResource&;
	auto get_pipeline(const std::string& key) -> Ref<Disarray::Pipeline>&;

	auto get_uniform_buffer_set() -> auto& { return uniform_buffer_set; }
	auto get_storage_buffer_set() -> auto& { return storage_buffer_set; }

	template <SceneFramebuffer Framebuffer> auto get_framebuffer() const { return framebuffers.at(Framebuffer); }

	void begin_execution();
	void submit_executed_commands();

	auto get_final_image() -> const Disarray::Image&;

private:
	const Disarray::Device& device;
	Scope<Renderer> renderer { nullptr };
	Extent renderer_extent {};

	Scope<Mesh> aabb_model;
	Ref<Disarray::CommandExecutor> command_executor {};

	std::unordered_map<SceneFramebuffer, Ref<Disarray::Framebuffer>> framebuffers {};

	using UniformSet = BufferSet<Disarray::UniformBuffer>;
	using StorageSet = BufferSet<Disarray::StorageBuffer>;
	UniformSet uniform_buffer_set;
	StorageSet storage_buffer_set;

	struct PointLightData {
		std::uint32_t calculate_point_lights { 0 };
		std::uint32_t use_gamma_correction { 0 };

		[[nodiscard]] static auto get_constants() -> std::vector<SpecialisationConstant>
		{
			return {
				SpecialisationConstant { ElementType::Uint },
				SpecialisationConstant { ElementType::Uint },
			};
		}
		[[nodiscard]] auto get_pointer() const -> const void* { return this; }
	};

	PointLightData point_light_data {};

	auto begin_pass(const Disarray::Framebuffer& framebuffer, bool explicit_clear) -> void;
	void clear_pass(RenderPasses render_pass);

	template <std::size_t Count> void clear_pass(std::array<RenderPasses, Count>&& render_passes)
	{
		for (const auto& pass : render_passes) {
			clear_pass(pass);
		}
	}
	auto draw_submesh(Disarray::CommandExecutor&, const Disarray::VertexBuffer&, const Disarray::IndexBuffer&, const Disarray::Pipeline&,
		const glm::vec4&, const glm::mat4&, PushConstant& push_constant) -> void;
};

} // namespace Disarray
