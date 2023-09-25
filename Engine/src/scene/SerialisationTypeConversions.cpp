#include "scene/SerialisationTypeConversions.hpp"

#include <magic_enum_flags.hpp>

#include "core/Tuple.hpp"
#include "graphics/ImageProperties.hpp"

namespace Disarray {

void to_json(nlohmann::json& j, const Extent& p)
{
	j = nlohmann::json {
		{ "width", p.width },
		{ "height", p.height },
	};
}
void from_json(const nlohmann::json& j, Extent& p)
{
	j.at("width").get_to(p.width);
	j.at("height").get_to(p.height);
}

void to_json(nlohmann::json& j, const FloatExtent& p)
{
	j = nlohmann::json {
		{ "width", p.width },
		{ "height", p.height },
	};
}
void from_json(const nlohmann::json& j, FloatExtent& p)
{
	j.at("width").get_to(p.width);
	j.at("height").get_to(p.height);
}

void to_json(nlohmann::json& output, const std::vector<LayoutElement>& elements)
{
	auto arr = json::array();
	for (const auto& layout : elements) {
		arr.push_back({ { "debug_name", layout.debug_name }, { "offset", layout.offset }, { "size", layout.size },
			{ "type", magic_enum::enum_name(layout.type) } });
	}
	output["elements"] = arr;
}

void from_json(const nlohmann::json& input, std::vector<LayoutElement>& elements)
{
	if (input.contains("elements")) {
		for (auto&& [index_string, value] : input["elements"].items()) {
			LayoutElement& element = elements.emplace_back();
			element.debug_name = value["debug_name"];
			element.offset = value["offset"];
			element.size = value["size"];
			element.type = *magic_enum::enum_cast<ElementType>(value["type"].get<std::string>());
		}
	}
}

void to_json(nlohmann::json& output, const VertexLayout& vertex_layout)
{
	json layout_object;
	layout_object["binding"] = { { "binding", vertex_layout.binding.binding },
		{ "input_rate", magic_enum::enum_name(vertex_layout.binding.input_rate) }, { "stride", vertex_layout.binding.stride } };
	layout_object["total_size"] = vertex_layout.total_size;

	layout_object["elements"] = vertex_layout.elements;
	output["vertex_layout"] = layout_object;
}

void from_json(const nlohmann::json& input, VertexLayout& vertex_layout)
{
	json layout_object;
	const auto& json_vertex_layout = input["vertex_layout"];
	if (json_vertex_layout.contains("binding")) {
		const auto& binding = json_vertex_layout["binding"];
		vertex_layout.binding = {
			.binding = binding["binding"].get<std::uint32_t>(),
			.stride = binding["stride"].get<std::uint32_t>(),
			.input_rate = magic_enum::enum_cast<InputRate>(binding["input_rate"].get<std::string>()).value_or(InputRate::Vertex),
		};
	}

	if (json_vertex_layout.contains("total_size")) {
		vertex_layout.total_size = json_vertex_layout["total_size"].get<std::uint32_t>();
	}

	std::vector<LayoutElement> elements = json_vertex_layout["elements"].get<std::vector<LayoutElement>>();
	vertex_layout.elements = std::move(elements);
	vertex_layout.calculate_layout();
}

void to_json(nlohmann::json& output, const PushConstantLayout& push_constant_layout)
{
	json object;
	object["size"] = push_constant_layout.size();

	auto arr = json::array();
	for (const auto& range : push_constant_layout.get_input_ranges()) {
		arr.push_back({ { "flags", magic_enum::enum_integer(range.flags) }, { "size", range.size }, { "offset", range.offset } });
	}
	object["ranges"] = arr;
	output["push_constant_layout"] = object;
}

void from_json(const nlohmann::json& input, PushConstantLayout& push_constant_layout)
{
	const auto& json_pc_layout = input["push_constant_layout"];
	if (json_pc_layout.contains("ranges")) {
		std::vector<PushConstantRange> ranges {};
		for (auto&& [index_string, value] : json_pc_layout["ranges"].items()) {
			PushConstantRange& range = ranges.emplace_back();
			range.size = value["size"].get<std::uint32_t>();
			range.flags = magic_enum::enum_flags_cast<PushConstantKind>(value["flags"].get<std::uint8_t>()).value_or(PushConstantKind::Both);
			range.offset = value["offset"].get<std::uint32_t>();
		}
		push_constant_layout.set_ranges(std::move(ranges));
	}
}

static constexpr auto construct_with(const auto& value, auto& output, ParameterType type)
{
	std::string_view enum_name = magic_enum::enum_name(type);
	output = { { "type", enum_name }, { "value", value } };
}

void to_json(nlohmann::json& json_output, const Parameter& parameter)
{
	std::visit(Tuple::overload {
				   [](const std::monostate& val) {},
				   [&json_object = json_output](const glm::vec2& val) { construct_with(val, json_object, ParameterType::Vec2); },
				   [&json_object = json_output](const glm::vec3& val) { construct_with(val, json_object, ParameterType::Vec3); },
				   [&json_object = json_output](const glm::vec4& val) { construct_with(val, json_object, ParameterType::Vec4); },
				   [&json_object = json_output](const float& val) { construct_with(val, json_object, ParameterType::Float); },
				   [&json_object = json_output](const double& val) { construct_with(val, json_object, ParameterType::Double); },
				   [&json_object = json_output](const int& val) { construct_with(val, json_object, ParameterType::Integer); },
				   [&json_object = json_output](const std::uint8_t& val) { construct_with(val, json_object, ParameterType::Uint8); },
				   [&json_object = json_output](const std::uint32_t& val) { construct_with(val, json_object, ParameterType::Uint32); },
				   [&json_object = json_output](const std::size_t& val) { construct_with(val, json_object, ParameterType::Uint64); },
				   [&json_object = json_output](const std::string& val) { construct_with(val, json_object, ParameterType::String); },
			   },
		parameter);
}

void from_json(const nlohmann::json& value, Parameter& parameter)
{
	if (!value.contains("type") || !value.contains("value")) {
		Log::error("From Json - Parameter", "Type or value was missing, json: {}", value.dump());
		return;
	}

	auto parameter_type = magic_enum::enum_cast<ParameterType>(value["type"].get<std::string>());

	if (!parameter_type.has_value()) {
		return;
	}

	switch (*parameter_type) {
	case ParameterType::Empty:
	default:
		return;
	case ParameterType::Float: {
		parameter = value["value"].get<float>();
		return;
	}
	case ParameterType::Double: {
		parameter = value["value"].get<double>();
		return;
	}
	case ParameterType::Integer: {
		parameter = value["value"].get<int>();
		return;
	}
	case ParameterType::Uint8: {
		parameter = value["value"].get<std::uint8_t>();
		return;
	}
	case ParameterType::Uint32: {
		parameter = value["value"].get<std::uint32_t>();
		return;
	}
	case ParameterType::Uint64: {
		parameter = value["value"].get<std::uint64_t>();
		return;
	}
	case ParameterType::String: {
		parameter = value["value"].get<std::string>();
		return;
	}
	case ParameterType::Vec2: {
		parameter = value["value"].get<glm::vec2>();
		return;
	}
	case ParameterType::Vec3: {
		parameter = value["value"].get<glm::vec3>();
		return;
	}
	case ParameterType::Vec4: {
		parameter = value["value"].get<glm::vec4>();
		return;
	}
	}
}

NLOHMANN_JSON_NAMESPACE_BEGIN
NLOHMANN_JSON_NAMESPACE_END

} // namespace Disarray
