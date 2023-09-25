#include "DisarrayPCH.hpp"

#include "scene/ComponentSerialisers.hpp"

#include <optional>

#include "core/Tuple.hpp"
#include "scene/CppScript.hpp"
#include "scene/SerialisationTypeConversions.hpp"

namespace Disarray {

using json = nlohmann::json;
using namespace std::string_view_literals;

template <class T> static constexpr auto assign_or_noop(auto& json_object, std::string_view key, const T& value)
{
	if (json_object.contains(key)) {
		return;
	}
	json_object[key] = value;
}

template <class T> static constexpr auto assign_or_noop(auto& json_object, std::string_view key, const std::optional<T>& value)
{
	if (!value.has_value()) {
		return;
	}
	assign_or_noop<T>(json_object, key, *value);
}

void PipelineSerialiser::serialise_impl(const Components::Pipeline& pipeline, nlohmann::json& object)
{
	if (pipeline.pipeline) {
		json properties;
		const auto& props = pipeline.pipeline->get_properties();
		properties["line_width"] = props.line_width;

		assign_or_noop(properties, "vertex_shader", props.vertex_shader->get_properties().path);
		assign_or_noop(properties, "fragment_shader", props.fragment_shader->get_properties().path);
		assign_or_noop(properties, "vertex_identifier", props.vertex_shader->get_properties().identifier);
		assign_or_noop(properties, "fragment_identifier", props.fragment_shader->get_properties().identifier);
		properties["vertex_layout"] = props.layout;
		properties["push_constant_layout"] = props.push_constant_layout;
		// PushConstantLayout push_constant_layout {};
		properties["extent"] = props.extent;
		properties["polygon_mode"] = magic_enum::enum_name(props.polygon_mode);
		properties["samples"] = magic_enum::enum_name(props.samples);
		properties["depth_comparison_operator"] = magic_enum::enum_name(props.depth_comparison_operator);
		properties["cull_mode"] = magic_enum::enum_name(props.cull_mode);
		properties["write_depth"] = props.write_depth;
		properties["test_depth"] = props.test_depth;
		object["properties"] = properties;
	}
}

void ScriptSerialiser::serialise_impl(const Components::Script& script, nlohmann::json& object)
{
	const auto& cpp_script = script.get_script();
	const auto& identifier = cpp_script.identifier();
	object["identifier"] = identifier;

	const auto& parameters = script.get_script().get_parameters();
	json params {};
	for (const auto& [param_key, value] : parameters) {
		params[param_key] = value;
	}
	object["parameters"] = params;
}

void MeshSerialiser::serialise_impl(const Components::Mesh& mesh, nlohmann::json& object)
{
	if (mesh.mesh) {
		json properties;
		const auto& props = mesh.mesh->get_properties();
		properties["path"] = props.path;
		properties["pipeline"] = static_cast<bool>(props.pipeline);
		properties["initial_rotation"] = props.initial_rotation;
		object["properties"] = properties;
	}
}

void TextureSerialiser::serialise_impl(const Components::Texture& texture, nlohmann::json& object)
{
	object["colour"] = texture.colour;

	if (texture.texture) {
		const TextureProperties& props = texture.texture->get_properties();
		json properties;
		properties["path"] = props.path;
		properties["extent"] = props.extent;
		properties["format"] = magic_enum::enum_name(props.format);
		properties["mips"] = props.mips ? *props.mips : 1;
		properties["path"] = props.path;
		properties["debug_name"] = props.debug_name;
		object["properties"] = properties;
	}
}

void InheritanceSerialiser::serialise_impl(const Components::Inheritance& inheritance, nlohmann::json& object)
{
	if (!inheritance.children.empty()) {
		object["children"] = inheritance.children;
	}
	if (inheritance.parent > 0) {
		object["parent"] = inheritance.parent;
	}
}

void TransformSerialiser::serialise_impl(const Components::Transform& transform, nlohmann::json& object)
{
	object["rotation"] = transform.rotation;
	object["position"] = transform.position;
	object["scale"] = transform.scale;
}

void LineGeometrySerialiser::serialise_impl(const Components::LineGeometry& geom, nlohmann::json& object)
{
	object["to_position"] = geom.to_position;
	object["geometry"] = magic_enum::enum_name(geom.geometry);
}

void QuadGeometrySerialiser::serialise_impl(const Components::QuadGeometry& geom, nlohmann::json& object)
{
	object["geometry"] = magic_enum::enum_name(geom.geometry);
}

} // namespace Disarray
