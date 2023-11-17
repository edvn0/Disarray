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

template <class Buffer> struct BufferSet {

	explicit BufferSet(const Disarray::Device& dev, const IGraphicsResource& resource, std::integral auto sets)
		: device(dev)
		, graphics_resource(resource)
		, frame_count(resource.get_image_count())
		, set_count(sets)
	{
	}

	auto add_buffer(std::size_t size, DescriptorSet set, DescriptorBinding binding) -> void
	{
		for (auto frame_index = FrameIndex {}; frame_index < frame_count; frame_index++) {
			const auto index = frame_index.value * frame_count + set.value * set_count + binding.value;
			storage_buffers.try_emplace(index,
				Buffer::construct_scoped(device,
					{
						.size = size,
						.always_mapped = true,
					}));
		}
	}

	template <class T> auto add_buffer(DescriptorSet set, DescriptorBinding binding) -> void { add_buffer(sizeof(T), set, binding); }

	auto for_frame(FrameIndex frame_index, DescriptorSet set, DescriptorBinding binding) -> Scope<Buffer>&
	{
		const auto index = frame_index.value * frame_count + set.value * set_count + binding.value;
		return storage_buffers.at(index);
	}

	auto for_frame(DescriptorSet set, DescriptorBinding binding) -> Scope<Buffer>&
	{
		const auto index = graphics_resource.get_current_frame_index().value * frame_count + set.value * set_count + binding.value;
		return storage_buffers.at(index);
	}

private:
	const Disarray::Device& device;
	const IGraphicsResource& graphics_resource;
	std::uint32_t frame_count { 3 };
	std::uint32_t set_count { 1 };
	std::unordered_map<std::uint32_t, Scope<Buffer>> storage_buffers {};
};

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

	auto draw_text(const Components::Transform&, const Components::Text&, const ColourVector&) -> void;

	template <SceneFramebuffer FB> void begin_pass(bool explicit_clear = false) { begin_pass(*get_framebuffer<FB>(), explicit_clear); }
	void end_pass();
	template <RenderPasses... RP> void clear_pass() { clear_pass(std::array<RenderPasses, sizeof...(RP)> { RP... }); };

	auto draw_identifiers(std::size_t count) -> void;
	auto draw_skybox() -> void;
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
	auto draw_aabb(const AABB& aabb, const ColourVector&, const TransformMatrix&) -> void;
	auto draw_static_submeshes(
		const Collections::ScopedStringMap<Disarray::Mesh>&, const Disarray::Pipeline&, const TransformMatrix&, const ColourVector&) -> void;
	auto draw_single_static_mesh(const Disarray::Mesh& mesh, const Disarray::Pipeline&, const TransformMatrix&, const ColourVector&) -> void;
	auto draw_single_static_mesh(const Disarray::VertexBuffer& vertices, const Disarray::IndexBuffer& indices, const Disarray::Pipeline&,
		const TransformMatrix&, const ColourVector&) -> void;
	auto draw_single_static_mesh(
		const Disarray::Mesh& mesh, const Disarray::Pipeline&, const Disarray::Material&, const TransformMatrix&, const ColourVector&) -> void;
	auto draw_single_static_mesh(const Disarray::VertexBuffer& vertices, const Disarray::IndexBuffer& indices, const Disarray::Pipeline&,
		const Disarray::Material&, const TransformMatrix&, const ColourVector&) -> void;
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

	auto get_entity_identifiers() -> auto& { return *storage_buffer_set->for_frame(DescriptorSet(0), DescriptorBinding(9)); }
	auto get_entity_transforms() -> auto& { return *storage_buffer_set->for_frame(DescriptorSet(0), DescriptorBinding(7)); }

	template <SceneFramebuffer Framebuffer> auto get_framebuffer() const { return framebuffers.at(Framebuffer); }

	void begin_execution();
	void submit_executed_commands();

	auto get_final_image() -> const Disarray::Image&;
	auto get_renderer() const -> const Scope<Renderer>& { return renderer; };

private:
	const Disarray::Device& device;
	Scope<Renderer> renderer { nullptr };
	Extent renderer_extent {};

	Scope<Mesh> aabb_model;
	Ref<Disarray::CommandExecutor> command_executor {};

	std::unordered_map<SceneFramebuffer, Ref<Disarray::Framebuffer>> framebuffers {};

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

	Scope<BufferSet<StorageBuffer>> storage_buffer_set {};
	Scope<BufferSet<UniformBuffer>> uniform_buffer_set {};
	PointLightData point_light_data {};

	Scope<Disarray::Material> default_material {};

	auto begin_pass(const Disarray::Framebuffer& framebuffer, bool explicit_clear) -> void;
	void clear_pass(RenderPasses render_pass);

	template <std::size_t Count> void clear_pass(std::array<RenderPasses, Count>&& render_passes)
	{
		for (const auto& pass : render_passes) {
			clear_pass(pass);
		}
	}
};

} // namespace Disarray
