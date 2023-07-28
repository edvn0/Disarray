#pragma once

#include "core/Types.hpp"
#include "graphics/Swapchain.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

namespace Disarray {

	enum class PolygonMode {
		Fill,
		Line,
		Point
	};

	enum class ElementType {
		Float,
		Float2,
		Float3,
		Float4,
	};

	static constexpr auto to_size(ElementType type)
	{
		switch (type) {
		case ElementType::Float:
			return sizeof(float) * 1;
		case ElementType::Float2:
			return sizeof(float) * 2;
		case ElementType::Float3:
			return sizeof(float) * 3;
		case ElementType::Float4:
			return sizeof(float) * 4;
		default:
			throw std::runtime_error("Could not map to size.");
		}
	}

	struct LayoutElement {
		constexpr LayoutElement(ElementType t)
			: type(t)
		{
			size = to_size(type);
		};

		ElementType type;
		std::size_t size { 0 };
		std::size_t offset { 0 };
	};

	struct VertexLayout {
		constexpr VertexLayout(std::initializer_list<LayoutElement> elems)
			: elements(elems)
		{
			for (auto& element : elements) {
				element.offset = total_size;
				total_size += element.size;
			}
		}

		VertexLayout(const VertexLayout& layout) {
			total_size = layout.total_size;
			elements = layout.elements;
		};

		std::size_t total_size { 0 };
		std::vector<LayoutElement> elements;
	};

	class Device;
	class Shader;
	class RenderPass;
	class Swapchain;

	struct PipelineProperties {
		Ref<Shader> vertex_shader { nullptr };
		Ref<Shader> fragment_shader { nullptr };
		Ref<RenderPass> render_pass {nullptr};
		VertexLayout layout;
		Extent extent {0,0};
		PolygonMode polygon_mode { PolygonMode::Fill };
	};

	class Pipeline {
	public:
		virtual ~Pipeline() = default;
		virtual void force_recreation() = 0;
		static Ref<Pipeline> construct(Ref<Device>, Ref<Disarray::Swapchain>, const PipelineProperties& = {});
	};

} // namespace Disarray