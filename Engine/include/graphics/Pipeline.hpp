#pragma once

#include "Forward.hpp"

#include <array>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#include "PushConstantLayout.hpp"
#include "core/DisarrayObject.hpp"
#include "core/Hashes.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/Shader.hpp"

using VkDescriptorSetLayout = struct VkDescriptorSetLayout_T*;

namespace Disarray {

enum class PipelineBindPoint : std::uint8_t {
	BindPointGraphics = 0,
	BindPointCompute = 1,
};
enum class PolygonMode : std::uint8_t { Fill, Line, Point };
enum class DepthCompareOperator : std::uint8_t {
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
enum class CullMode : std::uint8_t { Back, Front, None, Both };
enum class FaceMode : std::uint8_t { Clockwise, CounterClockwise };
enum class ElementType : std::uint8_t {
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
enum class VertexInput : std::uint8_t {
	Position,
	TextureCoordinates,
	Normals,
	Colour,
	Tangent,
	Bitangent,
};
static constexpr inline const std::array<VertexInput, 6> default_vertex_inputs {
	VertexInput::Position,
	VertexInput::TextureCoordinates,
	VertexInput::Normals,
	VertexInput::Colour,
	VertexInput::Tangent,
	VertexInput::Bitangent,
};

static constexpr auto to_size(ElementType type) -> std::size_t
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
		return sizeof(std::uint32_t);
	default:
		unreachable("Could not map to size.");
	}
}

struct LayoutElement {
	LayoutElement(ElementType element_type, std::string debug = "Empty")
		: type(element_type)
		, debug_name(std::move(debug))
		, size(to_size(type)) {

		};

	ElementType type;
	std::string debug_name;
	std::uint32_t size { 0 };
	std::uint32_t offset { 0 };
};

enum class InputRate : std::uint8_t { Vertex, Instance };

struct VertexBinding {
	std::uint32_t binding { 0 };
	std::uint32_t stride { 0 };
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

	[[nodiscard]] auto construct_binding() const -> const VertexBinding& { return binding; }

	std::uint32_t total_size { 0 };
	std::vector<LayoutElement> elements;
	VertexBinding binding;
};

struct SpecialisationConstant {
	std::uint32_t size;
	std::uint32_t id { 0 };
	std::uint32_t offset { 0 };

	explicit constexpr SpecialisationConstant(ElementType type)
		: size(to_size(type))
	{
	}
};

template <class Describer>
concept SpecialisationConstantDescriber = requires(Describer describer) {
	{
		Describer::get_constants()
	} -> std::same_as<std::vector<SpecialisationConstant>>;
	{
		describer.get_pointer()
	} -> std::same_as<const void*>;
};

struct SpecialisationConstantDescription {
	std::vector<SpecialisationConstant> constants {};
	std::uint32_t total_size { 0 };
	const void* data { nullptr };

	constexpr SpecialisationConstantDescription() = default;

	template <SpecialisationConstantDescriber Describer>
	explicit SpecialisationConstantDescription(const Describer& describer)
		: constants(Describer::get_constants())
		, data(describer.get_pointer())
	{
		if constexpr (requires {
						  {
							  Describer::get_identifiers()
						  } -> std::same_as<std::vector<std::uint32_t>>;
					  }) {
			compute_total_size(Describer::get_identifiers());
		} else {
			compute_total_size();
		}
	}

	[[nodiscard]] auto valid() const { return total_size > 0 && data != nullptr && !constants.empty(); }

private:
	static constexpr auto make_sequenced(std::size_t count)
	{
		std::vector<std::uint32_t> output;
		output.resize(count);
		for (auto i = 0ULL; i < count; i++) {
			output.at(i) = i;
		}
		return output;
	}
	constexpr void compute_total_size(const std::vector<std::uint32_t>& identifiers)
	{
		std::uint32_t offset = 0;
		std::size_t identifier = 0;
		for (auto& constant : constants) {
			constant.id = identifiers.at(identifier++);
			constant.offset = offset;
			total_size += static_cast<std::uint32_t>(constant.size);
			offset += total_size;
		}
	}
	constexpr void compute_total_size()
	{
		const auto count = constants.size();
		auto identifiers = make_sequenced(count);
		std::uint32_t offset = 0;
		std::size_t identifier = 0;
		for (auto& constant : constants) {
			constant.id = identifiers.at(identifier++);
			constant.offset = offset;
			total_size += constant.size;
			offset += total_size;
		}
	}
};

struct PipelineProperties {
	Ref<Shader> vertex_shader { nullptr };
	Ref<Shader> fragment_shader { nullptr };
	Ref<Shader> compute_shader { nullptr };
	Ref<Framebuffer> framebuffer { nullptr };
	VertexLayout layout {};
	PushConstantLayout push_constant_layout {};
	Extent extent { 0, 0 };
	PolygonMode polygon_mode { PolygonMode::Fill };
	float line_width { 1.0F };
	SampleCount samples { SampleCount::One };
	DepthCompareOperator depth_comparison_operator { DepthCompareOperator::Less };
	CullMode cull_mode { CullMode::Back };
	FaceMode face_mode { FaceMode::CounterClockwise };
	bool write_depth { true };
	bool test_depth { true };
	std::vector<VkDescriptorSetLayout> descriptor_set_layouts {};
	SpecialisationConstantDescription specialisation_constants {};

	auto hash() const -> std::size_t
	{
		std::size_t seed { 0 };
		hash_combine(
			seed, extent.width, extent.height, cull_mode, face_mode, depth_comparison_operator, samples, write_depth, test_depth, line_width);

		if (vertex_shader) {
			hash_combine(seed, *vertex_shader);
		}

		if (fragment_shader) {
			hash_combine(seed, *fragment_shader);
		}
		return seed;
	}

	void set_shader_with_type(ShaderType type, const Ref<Disarray::Shader>& shader)
	{
		switch (type) {
		case ShaderType::Vertex: {
			vertex_shader = shader;
			break;
		}
		case ShaderType::Fragment: {
			fragment_shader = shader;
			break;
		}
		case ShaderType::Compute: {
			compute_shader = shader;
			break;
		}
		case ShaderType::Include:
			break;
		}
	}

	auto is_valid() const -> bool { return !descriptor_set_layouts.empty() && framebuffer != nullptr; }
};

class Pipeline : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(Pipeline, PipelineProperties)
public:
	virtual auto get_render_pass() -> Disarray::RenderPass& = 0;
	virtual auto get_framebuffer() -> Disarray::Framebuffer& = 0;
	virtual auto get_render_pass() const -> const Disarray::RenderPass& = 0;
	virtual auto get_framebuffer() const -> const Disarray::Framebuffer& = 0;

	[[nodiscard]] auto is_valid() const -> bool { return props.is_valid(); };

	[[nodiscard]] auto has_shader_with_name(std::string_view name) const -> bool
	{
		const auto& vert = get_properties().vertex_shader->get_properties().identifier;
		const auto& frag = get_properties().fragment_shader->get_properties().identifier;
		return vert.string() == name || frag.string() == name;
	}
	[[nodiscard]] auto has_shader_with_name(const std::filesystem::path& name) const -> bool
	{
		const auto& vert = get_properties().vertex_shader->get_properties().identifier;
		const auto& frag = get_properties().fragment_shader->get_properties().identifier;
		return vert.string() == name || frag.string() == name;
	}

	auto operator==(const Pipeline& other) const -> bool { return other.get_properties().hash() == get_properties().hash(); }
};

} // namespace Disarray
