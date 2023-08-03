#pragma once

#include "Forward.hpp"
#include "PushConstantLayout.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Swapchain.hpp"

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace Disarray {

	enum class PolygonMode { Fill, Line, Point };

	enum class ElementType {
		Float,
		Double,
		Float2,
		Float3,
		Float4,
		Int2,
		Int3,
		Int4,
		Uint2,
		Uint3,
		Uint4,
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
		LayoutElement(ElementType t, const std::string& debug = "Empty")
			: type(t)
			, debug_name(debug)
		{
			size = to_size(type);
		};

		ElementType type;
		std::string debug_name;
		std::uint32_t size { 0 };
		std::uint32_t offset { 0 };
	};

	enum class InputRate { Vertex, Instance };

	struct VertexBinding {
		std::uint32_t binding { 0 };
		std::uint32_t stride {};
		InputRate input_rate { InputRate::Vertex };
	};

	struct VertexLayout {
		VertexLayout(std::initializer_list<LayoutElement> elems, const VertexBinding& bind = {})
			: elements(elems)
			, binding(bind)
		{
			for (auto& element : elements) {
				element.offset = total_size;
				total_size += element.size;
			}
			binding.stride = total_size;
		}

		VertexLayout(const VertexLayout& layout)
		{
			total_size = layout.total_size;
			elements = layout.elements;
			binding = layout.binding;
		};

		const VertexBinding& construct_binding() { return binding; }

		std::uint32_t total_size { 0 };
		std::vector<LayoutElement> elements;
		VertexBinding binding;
	};

	class Device;
	class Shader;
	class Swapchain;

	struct PipelineProperties {
		Ref<Shader> vertex_shader { nullptr };
		Ref<Shader> fragment_shader { nullptr };
		Ref<Framebuffer> framebuffer { nullptr };
		VertexLayout layout {};
		PushConstantLayout push_constant_layout {};
		Extent extent { 0, 0 };
		PolygonMode polygon_mode { PolygonMode::Fill };
		float line_width { 1.0f };
	};

	class Pipeline : public ReferenceCountable {
		DISARRAY_MAKE_REFERENCE_COUNTABLE(Pipeline)
	public:
		virtual void force_recreation() = 0;

		virtual Disarray::RenderPass& get_render_pass() = 0;
		virtual Disarray::Framebuffer& get_framebuffer() = 0;

		virtual void recreate(bool should_clear) = 0;

		static Ref<Pipeline> construct(Disarray::Device&, Disarray::Swapchain&, const PipelineProperties&);
	};

} // namespace Disarray
