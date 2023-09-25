#include "core/Collections.hpp"
#include "scene/ComponentSerialisers.hpp"
#include "scene/Scripts.hpp"
#include "scene/SerialisationTypeConversions.hpp"

namespace Disarray {

using json = nlohmann::json;
using namespace std::string_view_literals;

auto PipelineDeserialiser::should_add_component_impl(const nlohmann::json& object) -> bool { return object.contains("properties"); }
void PipelineDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Pipeline& pipeline, const Device& device)
{
	using Path = std::filesystem::path;
	auto props = object["properties"];

	PipelineProperties properties {
		.vertex_shader = Shader::compile(device, Path { props["vertex_shader"].get<std::string>() }),
		.fragment_shader = Shader::compile(device, Path { props["fragment_shader"].get<std::string>() }),
		// framebuffer,
		.layout = props["vertex_layout"],
		.push_constant_layout = props["push_constant_layout"],
		.extent = props["extent"],
		.polygon_mode = to_enum_value<PolygonMode>(props, "polygon_mode").value_or(PolygonMode::Fill),
		.line_width = props["line_width"],
		.samples = to_enum_value<SampleCount>(props, "samples").value_or(SampleCount::One),
		.depth_comparison_operator
		= to_enum_value<DepthCompareOperator>(props, "depth_comparison_operator").value_or(DepthCompareOperator::GreaterOrEqual),
		.cull_mode = to_enum_value<CullMode>(props, "cull_mode").value_or(CullMode::Back),
		.write_depth = props["write_depth"],
		.test_depth = props["test_depth"],
		// descriptor_set_layout { nullptr ,
		// descriptor_set_layout_count { 0 ,
	};

	pipeline.pipeline = Pipeline::construct(device, properties);
}

auto ScriptDeserialiser::should_add_component_impl(const nlohmann::json& object) -> bool { return object.contains("identifier"); }
void ScriptDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Script& script, const Device& device)
{
	const auto& identifier = object["identifier"];
	static Collections::StringMap<Scope<CppScript>> scripts {};
	const auto& s = scripts[identifier];

	Collections::StringViewMap<Parameter> parameters {};
	
	if (object.contains("parameters")) {
		const json& json_parameters = object["parameters"];

		for (auto&& [key, val] : json_parameters.items()) {
			parameters.try_emplace(key, val);
		}
	}

	if (identifier == "MoveInCircle") {
		script.bind<Scripts::MoveInCircleScript>(parameters);
	}

	if (identifier == "LinearMovement") {
		script.bind<Scripts::LinearMovementScript>(parameters);
	}
}

auto MeshDeserialiser::should_add_component_impl(const nlohmann::json& object) -> bool { return object.contains("properties"); }
void MeshDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Mesh& mesh, const Device& device)
{

	auto props = object["properties"];
	MeshProperties properties {
		.path = props["path"],
		.pipeline = nullptr,
		.initial_rotation = props["initial_rotation"],
	};

	mesh.mesh = Mesh::construct(device, properties);
}

auto TextureDeserialiser::should_add_component_impl(const nlohmann::json& object) -> bool { return object.contains("properties"); }
void TextureDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Texture& texture, const Device& device)
{
	auto props = object["properties"];
	TextureProperties properties {
		.extent = props["extent"],
		.format = to_enum_value<ImageFormat>(props, "format").value_or(ImageFormat::SBGR),
		.mips = props["mips"],
		.path = props["path"],
		.debug_name = props["debug_name"],
	};
	texture.texture = Texture::construct(device, properties);
	texture.colour = object["colour"];
}

auto InheritanceDeserialiser::should_add_component_impl(const nlohmann::json& object) -> bool
{
	return object.contains("children") || object.contains("parent");
}
void InheritanceDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Inheritance& inheritance, const Device&)
{
	if (object.contains("children")) {
		std::unordered_set<Identifier> children;
		object["children"].get_to(children);
		inheritance.children = std::move(children);
	}

	if (object.contains("parent")) {
		inheritance.parent = object["parent"];
	}
}

auto TransformDeserialiser::should_add_component_impl(const nlohmann::json& object_for_the_component) -> bool { return true; }
void TransformDeserialiser::deserialise_impl(const nlohmann::json& object, Components::Transform& transform, const Device&)
{
	transform.rotation = object["rotation"];
	transform.position = object["position"];
	transform.scale = object["scale"];
}

auto LineGeometryDeserialiser::should_add_component_impl(const nlohmann::json& object_for_the_component) -> bool
{
	return object_for_the_component.contains("to_position") && object_for_the_component.contains("geometry");
}
void LineGeometryDeserialiser::deserialise_impl(const nlohmann::json& object, Components::LineGeometry& geom, const Device&)
{
	auto enum_val = to_enum_value<Geometry>(object, "geometry"sv);
	geom.geometry = enum_val.value_or(Geometry::Line);
	geom.to_position = object["to_position"];
}

auto QuadGeometryDeserialiser::should_add_component_impl(const nlohmann::json& object_for_the_component) -> bool
{
	return object_for_the_component.contains("geometry");
}
void QuadGeometryDeserialiser::deserialise_impl(const nlohmann::json& object, Components::QuadGeometry& geom, const Device&)
{
	auto enum_val = to_enum_value<Geometry>(object, "geometry"sv);
	geom.geometry = enum_val.value_or(Geometry::Rectangle);
}

} // namespace Disarray
