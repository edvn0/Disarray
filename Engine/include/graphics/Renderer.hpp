#pragma once

#include "Forward.hpp"
#include "core/Types.hpp"
#include "core/UsageBadge.hpp"
#include "graphics/Mesh.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/PipelineCache.hpp"
#include "graphics/Swapchain.hpp"
#include "graphics/Texture.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <optional>

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
		glm::vec3 position {};
		glm::vec3 scale { 1.0 };
		glm::vec3 to_position {};
		glm::quat rotation { glm::identity<glm::quat>() };
		std::optional<glm::vec3> dimensions { std::nullopt };
		std::optional<float> radius { std::nullopt };

		template <Geometry T> bool valid()
		{
			if constexpr (T == Geometry::Circle) {
				return radius.has_value() && !dimensions.has_value();
			}
			if constexpr (T == Geometry::Triangle || T == Geometry::Rectangle) {
				return dimensions.has_value() && !radius.has_value();
			}
			return false;
		}

		auto to_transform() const
		{
			return glm::translate(glm::mat4 { 1.0f }, position) * glm::scale(glm::mat4 { 1.0f }, scale) * glm::mat4_cast(rotation);
			;
		}
	};

	class IGraphics {
	public:
		virtual ~IGraphics() = default;

		virtual void draw_planar_geometry(Geometry, const GeometryProperties&) = 0;
		virtual void draw_mesh(Disarray::CommandExecutor&, Disarray::Mesh&, const GeometryProperties& = {}) = 0;
		virtual void submit_batched_geometry(Disarray::CommandExecutor&) = 0;
	};

	class IGraphicsResource {
	public:
		virtual ~IGraphicsResource() = default;

		virtual void expose_to_shaders(Image&) = 0;
		virtual void expose_to_shaders(Texture& tex) { expose_to_shaders(tex.get_image()); };
		virtual VkDescriptorSet get_descriptor_set(std::uint32_t) = 0;
		virtual VkDescriptorSet get_descriptor_set() = 0;
		virtual VkDescriptorSetLayout get_descriptor_set_layout() = 0;
		virtual std::uint32_t get_descriptor_set_layout_count() = 0;
	};

	class Renderer : public IGraphics, public IGraphicsResource, public ReferenceCountable {
	public:
		virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&, bool explicit_clear) = 0;
		virtual void begin_pass(Disarray::CommandExecutor&, Disarray::Framebuffer&) = 0;
		virtual void begin_pass(Disarray::CommandExecutor&) = 0;
		virtual void end_pass(Disarray::CommandExecutor&) = 0;

		virtual void on_resize() = 0;
		virtual PipelineCache& get_pipeline_cache() = 0;

		virtual void begin_frame(UsageBadge<App>) = 0;
		virtual void end_frame(UsageBadge<App>) = 0;

		virtual void force_recreation() = 0;

		static Ref<Renderer> construct(Disarray::Device&, Disarray::Swapchain&, const RendererProperties&);
	};

} // namespace Disarray
