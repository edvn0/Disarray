#pragma once

#include "Forward.hpp"
#include "core/Types.hpp"
#include "core/UniquelyIdentifiable.hpp"
#include "core/UsageBadge.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/Texture.hpp"
#include "graphics/TextureCache.hpp"

#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <optional>
#include <scene/Camera.hpp>

using VkDescriptorSet = struct VkDescriptorSet_T*;
using VkDescriptorSetLayout = struct VkDescriptorSetLayout_T*;

namespace Disarray {

	struct RendererProperties {
		std::string debug_name { "Unknown" };
	};

	struct Extent;

	enum class Geometry {
		Circle,
		Triangle,
		Rectangle,
		Line,
	};

	struct GeometryProperties {
		glm::vec3 position { 0.0f };
		glm::vec3 scale { 1.0 };
		glm::vec3 to_position { 0.0f };
		glm::vec4 colour { 1.0f };
		glm::quat rotation { glm::identity<glm::quat>() };
		glm::vec3 dimensions { 1.f };
		std::optional<std::uint32_t> identifier { std::nullopt };
		std::optional<float> radius { std::nullopt };

		template <Geometry T> bool valid()
		{
			if constexpr (T == Geometry::Circle) {
				return radius.has_value();
			}
			if constexpr (T == Geometry::Triangle || T == Geometry::Rectangle) {
				return !radius.has_value();
			}
			return false;
		}

		auto to_transform() const
		{
			return glm::translate(glm::mat4 { 1.0f }, position) * glm::scale(glm::mat4 { 1.0f }, scale) * glm::mat4_cast(rotation);
		}
	};

	struct PushConstant {
		glm::mat4 object_transform { 1.0f };
		glm::vec4 colour { 1.0f };
		std::uint32_t max_identifiers {};
	};

	struct UBO {
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 view_projection;
	};

	class IGraphics {
	public:
		virtual ~IGraphics() = default;

		virtual void draw_planar_geometry(Geometry, const GeometryProperties&) = 0;
		virtual void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const GeometryProperties& = {}) = 0;
		virtual void draw_mesh(Disarray::CommandExecutor&, const Disarray::Mesh&, const glm::mat4& transform = glm::identity<glm::mat4>()) = 0;
		virtual void draw_mesh(
			Disarray::CommandExecutor&, const Disarray::Mesh&, const Disarray::Pipeline&, const glm::mat4& transform = glm::identity<glm::mat4>())
			= 0;
		virtual void submit_batched_geometry(Disarray::CommandExecutor&) = 0;
		virtual void on_batch_full(std::function<void(Renderer&)>&&) = 0;
		virtual void flush_batch(Disarray::CommandExecutor&) = 0;
	};

	class IGraphicsResource {
	public:
		virtual ~IGraphicsResource() = default;

		virtual void expose_to_shaders(Image&) = 0;
		void expose_to_shaders(Texture& tex) { expose_to_shaders(tex.get_image()); };
		virtual VkDescriptorSet get_descriptor_set(std::uint32_t) = 0;
		virtual VkDescriptorSet get_descriptor_set() = 0;
		virtual const std::vector<VkDescriptorSetLayout>& get_descriptor_set_layouts() = 0;
		virtual const PushConstant* get_push_constant() const = 0;
		virtual PushConstant& get_editable_push_constant() = 0;
	};

	class Layer;

	class Renderer : public IGraphics, public IGraphicsResource, public ReferenceCountable {
	public:
		virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&, bool explicit_clear) = 0;
		virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&) = 0;
		virtual void begin_pass(Disarray::CommandExecutor&) = 0;
		virtual void end_pass(Disarray::CommandExecutor&) = 0;

		virtual void on_resize() = 0;

		virtual PipelineCache& get_pipeline_cache() = 0;
		virtual TextureCache& get_texture_cache() = 0;

		virtual void begin_frame(Camera& camera) = 0;
		virtual void end_frame() = 0;

		virtual void force_recreation() = 0;

		static Ref<Renderer> construct(Disarray::Device&, Disarray::Swapchain&, const RendererProperties&);
	};

} // namespace Disarray
