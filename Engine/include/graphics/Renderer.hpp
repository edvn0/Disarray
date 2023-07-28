#pragma once

#include "core/Types.hpp"
#include "graphics/Swapchain.hpp"

#include <glm/glm.hpp>
#include <optional>

namespace Disarray {

	struct RendererProperties {};

	class Device;
	class CommandExecutor;
	class RenderPass;
	class Pipeline;
	class Framebuffer;

	struct Extent;

	enum class Geometry {
		Circle,
		Triangle,
		Rectangle,
	};

	struct GeometryProperties {
		glm::vec3 position {};
		std::optional<glm::vec3> dimensions{std::nullopt};
		std::optional<float> radius {std::nullopt};

		template<Geometry T>
		bool valid(){
			if constexpr (T == Geometry::Circle) {
				return radius.has_value() && !dimensions.has_value();
			}
			if constexpr (T == Geometry::Triangle || T == Geometry::Rectangle) {
				return dimensions.has_value() && !radius.has_value();
			}
			return false;
		}
	};

	class IGraphics {
	public:
		virtual ~IGraphics() = default;

		virtual void draw_planar_geometry(Ref<Disarray::CommandExecutor>, Geometry, const GeometryProperties&) = 0;
	};

	class Renderer: public IGraphics {
	public:
		virtual ~Renderer() = default;

		virtual void begin_pass(Ref<Disarray::CommandExecutor>, Ref<Disarray::RenderPass>, Ref<Disarray::Pipeline>, Ref<Disarray::Framebuffer>) = 0;
		virtual void end_pass(Ref<Disarray::CommandExecutor>) = 0;

		virtual void set_extent(const Extent&) = 0;

		static Ref<Renderer> construct(Ref<Device>, Ref<Swapchain>, const RendererProperties&);
	};

}