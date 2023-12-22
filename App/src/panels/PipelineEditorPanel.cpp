#include "panels/PipelineEditorPanel.hpp"

namespace Disarray::Client {

PipelineEditorPanel::PipelineEditorPanel(Device& dev, Window&, Swapchain&, PipelineCache& cache)
	: pipeline_cache(cache)
	, device(dev)
{
}

void PipelineEditorPanel::update(float time_step) { }

void PipelineEditorPanel::interface()
{
	UI::Scope scope("PipelineEditorPanel");

	auto new_value = preview.preview_value;
	if (UI::begin_combo("Pipeline", preview.preview_value == nullptr ? "Empty" : preview.name)) {
		pipeline_cache.for_each_in_storage([&](auto&& kv) {
			auto&& [key, pipeline] = kv;
			const bool is_selected = (pipeline == preview.preview_value);
			if (UI::is_selectable(key, is_selected)) {
				new_value = pipeline;
				preview.name = key;
			}

			if (is_selected) {
				UI::set_item_default_focus();
			}
		});
		UI::end_combo();
	}

	if (new_value != nullptr) {
		auto& props = new_value->get_properties();
		bool any_changed = false;
		any_changed |= UI::combo_choice<DepthCompareOperator>("Compare operator", std::ref(props.depth_comparison_operator));
		any_changed |= UI::combo_choice<CullMode>("Cull mode", std::ref(props.cull_mode));
		any_changed |= UI::combo_choice<FaceMode>("Face mode", std::ref(props.face_mode));
		any_changed |= UI::combo_choice<PolygonMode>("Polygon mode", std::ref(props.polygon_mode));

		if (props.is_single_shader()) {
			any_changed |= UI::shader_drop_button(device, "Single Shader", std::ref(props.single_shader));
		} else {

			any_changed |= UI::shader_drop_button(device, "Vertex Shader", ShaderType::Vertex, std::ref(props.vertex_shader));
			any_changed |= UI::shader_drop_button(device, "Fragment Shader", ShaderType::Fragment, std::ref(props.fragment_shader));
		}

		any_changed |= UI::checkbox("Depth test", props.test_depth);
		any_changed |= UI::checkbox("Depth write", props.write_depth);

		if (any_changed) {
			new_value->recreate(true, {});
		}
		preview.preview_value = new_value;
	}
}

} // namespace Disarray::Client
