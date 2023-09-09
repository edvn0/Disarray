#pragma once

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#include "Forward.hpp"
#include "PushConstantLayout.hpp"
#include "core/DisarrayObject.hpp"
#include "core/Hashes.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Swapchain.hpp"

using VkDescriptorSetLayout = struct VkDescriptorSetLayout_T*;

namespace Disarray {

enum class PolygonMode { Fill, Line, Point };

enum class DepthCompareOperator {
	None = 0,
	Never,
	NotEqual,
	Less,
	LessOrEqual,
	Greater,
	GreaterOrEqual,
	Equal,
	Always,
};

enum class CullMode { Back, Front, None, Both };
enum class FaceMode { Clockwise, CounterClockwise };

enum class ElementType {
	Float,
	Double,
	Float2,
	Float3,
	Float4,
	Int2,
	Int3,
	Int4,
	Uint,
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
	case ElementType::Uint:
		return sizeof(unsigned);
	default:
		unreachable("Could not map to size.");
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
	Ref<Shader> compute_shader { nullptr };
	Ref<Framebuffer> framebuffer { nullptr };
	VertexLayout layout {};
	PushConstantLayout push_constant_layout {};
	Extent extent { 0, 0 };
	PolygonMode polygon_mode { PolygonMode::Fill };
	float line_width { 1.0f };
	SampleCount samples { SampleCount::One };
	DepthCompareOperator depth_comparison_operator { DepthCompareOperator::Less };
	CullMode cull_mode { CullMode::Front };
	FaceMode face_mode { FaceMode::Clockwise };
	bool write_depth { true };
	bool test_depth { true };
	std::vector<VkDescriptorSetLayout> descriptor_set_layouts;

	std::size_t hash() const
	{
		std::size_t seed { 0 };
		hash_combine(
			seed, extent.width, extent.height, cull_mode, face_mode, depth_comparison_operator, samples, write_depth, test_depth, line_width);
		return seed;
	}
};

class Pipeline : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(Pipeline, PipelineProperties)
public:
	virtual auto get_render_pass() -> Disarray::RenderPass& = 0;
	virtual auto get_framebuffer() -> Disarray::Framebuffer& = 0;

	[[nodiscard]] auto has_shader_with_name(std::string_view name) const -> bool
	{
		auto vert = get_properties().vertex_shader->get_properties().identifier;
		auto frag = get_properties().fragment_shader->get_properties().identifier;
		return vert.string() == name || frag.string() == name;
	}
	[[nodiscard]] auto has_shader_with_name(const std::filesystem::path& name) const -> bool
	{
		auto vert = get_properties().vertex_shader->get_properties().identifier;
		auto frag = get_properties().fragment_shader->get_properties().identifier;
		return vert.string() == name || frag.string() == name;
	}
};

} // namespace Disarray
