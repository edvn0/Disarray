#pragma once

#include "Forward.hpp"

#include <glm/fwd.hpp>

#include "core/Collections.hpp"
#include "core/PointerDefinition.hpp"
#include "core/Types.hpp"
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

	auto draw_text(const std::string& text_data, const glm::uvec2& position, float size, const glm::vec4& colour) -> void;
	auto draw_text(const std::string& text_data, const glm::mat4& transform, float size, const glm::vec4& colour) -> void;

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
	auto draw_point_lights(const Disarray::Mesh& point_light_mesh, std::uint32_t count, const Disarray::Pipeline& pipeline) -> void;
	auto draw_planar_geometry(Geometry, const GeometryProperties&) -> void;
	auto draw_aabb(const AABB& aabb, const glm::vec4& colour, const glm::mat4& transform) -> void;
	auto draw_static_submeshes(const Collections::ScopedStringMap<Disarray::MeshSubstructure>&, const Disarray::Pipeline&, const glm::mat4& transform,
		const glm::vec4& colour) -> void;
	auto draw_single_static_mesh(const Disarray::Mesh& mesh, const Disarray::Pipeline& pipeline, const glm::mat4& transform, const glm::vec4& colour)
		-> void;
	auto draw_single_static_mesh(const Disarray::VertexBuffer& vertices, const Disarray::IndexBuffer& indices, const Disarray::Pipeline& pipeline,
		const glm::mat4& transform, const glm::vec4& colour) -> void;
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

	auto get_point_light_transforms() -> auto& { return *point_light_transforms; }
	auto get_point_light_colours() -> auto& { return *point_light_colours; }
	auto get_entity_identifiers() -> auto& { return *entity_identifiers; }
	auto get_entity_transforms() -> auto& { return *entity_transforms; }

	template <SceneFramebuffer Framebuffer> auto get_framebuffer() const { return framebuffers.at(Framebuffer); }

	void begin_execution();
	void submit_executed_commands();

	template <class Buffer> auto begin_uniform_transaction() -> decltype(auto)
	{
		constexpr auto identifier = identifier_for<Buffer>;

		if constexpr (identifier == UBOIdentifier::Default) {
			return uniform->transaction();
		} else if constexpr (identifier == UBOIdentifier::Camera) {
			return camera_ubo->transaction();
		} else if constexpr (identifier == UBOIdentifier::PointLight) {
			return lights->transaction();
		} else if constexpr (identifier == UBOIdentifier::ShadowPass) {
			return shadow_pass_ubo->transaction();
		} else if constexpr (identifier == UBOIdentifier::DirectionalLight) {
			return directional_light_ubo->transaction();
		} else if constexpr (identifier == UBOIdentifier::Glyph) {
			return glyph_ubo->transaction();
		} else if constexpr (identifier == UBOIdentifier::SpotLight) {
			return spot_light_data->transaction();
		} else {
			static_assert(identifier_for<Buffer> == UBOIdentifier::Missing, "What???");
		}
	}

	auto get_final_image() -> const Disarray::Image&;

private:
	const Disarray::Device& device;
	Scope<Renderer> renderer { nullptr };
	Extent renderer_extent {};

	Scope<Mesh> aabb_model;
	Ref<Disarray::CommandExecutor> command_executor {};

	std::unordered_map<SceneFramebuffer, Ref<Disarray::Framebuffer>> framebuffers {};

	Scope<Disarray::StorageBuffer> point_light_transforms {};
	Scope<Disarray::StorageBuffer> point_light_colours {};
	Scope<Disarray::StorageBuffer> entity_identifiers {};
	Scope<Disarray::StorageBuffer> entity_transforms {};

	struct PointLightData {
		std::uint32_t calculate_point_lights { 1 };
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

	Scope<UniformBufferSet<UBO>> uniform {};
	Scope<UniformBufferSet<CameraUBO>> camera_ubo {};
	Scope<UniformBufferSet<PointLights>> lights {};
	Scope<UniformBufferSet<ShadowPassUBO>> shadow_pass_ubo {};
	Scope<UniformBufferSet<DirectionalLightUBO>> directional_light_ubo {};
	Scope<UniformBufferSet<GlyphUBO>> glyph_ubo {};
	Scope<UniformBufferSet<SpotLights>> spot_light_data {};

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
