#include "DisarrayPCH.hpp"

#include "scene/Serialiser.hpp"

namespace Disarray {

	using json = nlohmann::json;

	void PipelineSerialiser::serialise_impl(const Components::Pipeline& pipeline, nlohmann::json& object)
	{
		const auto& props = pipeline.pipeline->get_properties();
		object["component_name"] = "Pipeline";
		object["line_width"] = props.line_width;
		object["vertex_shader"] = props.vertex_shader->path();
		object["fragment_shader"] = props.fragment_shader->path();
		// VertexLayout layout {};
		// PushConstantLayout push_constant_layout {};
		object["extent"] = json::array({ props.extent.width, props.extent.height });
		object["polygon_mode"] = magic_enum::enum_name(props.polygon_mode);
		object["samples"] = magic_enum::enum_name(props.samples);
		object["depth_comparison_operator"] = magic_enum::enum_name(props.depth_comparison_operator);
		object["cull_mode"] = magic_enum::enum_name(props.cull_mode);
		object["write_depth"] = props.write_depth;
		object["test_depth"] = props.test_depth;
	}

} // namespace Disarray
