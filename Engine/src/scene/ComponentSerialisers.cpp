#include "DisarrayPCH.hpp"

#include "scene/ComponentSerialisers.hpp"

#include "scene/SerialisationTypeConversions.hpp"

namespace Disarray {

using json = nlohmann::json;
using namespace std::string_view_literals;

void PipelineSerialiser::serialise_impl(const Components::Pipeline& pipeline, nlohmann::json& object)
{
	if (pipeline.pipeline) {
		json properties;
		const auto& props = pipeline.pipeline->get_properties();
		properties["line_width"] = props.line_width;
		properties["vertex_shader"] = props.vertex_shader->get_properties().path;
		properties["fragment_shader"] = props.fragment_shader->get_properties().path;
		properties["vertex_layout"] = [](const VertexLayout& vertex_layout) {
			json layout_object;
			layout_object["binding"] = { { "binding", vertex_layout.binding.binding },
				{ "input_rate", magic_enum::enum_name(vertex_layout.binding.input_rate) }, { "stride", vertex_layout.binding.stride } };
			layout_object["total_size"] = vertex_layout.total_size;

			auto arr = json::array();
			for (const auto& layout : vertex_layout.elements) {
				arr.push_back({ { "debug_name", layout.debug_name }, { "offset", layout.offset }, { "size", layout.size },
					{ "type", magic_enum::enum_name(layout.type) } });
			}
			layout_object["elements"] = arr;
			return layout_object;
		}(props.layout);
		properties["push_constant_layout"] = [](const PushConstantLayout& push_constant_layout) { return json(); }(props.push_constant_layout);
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
