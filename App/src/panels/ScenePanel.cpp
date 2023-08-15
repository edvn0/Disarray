#include "panels/ScenePanel.hpp"

#include "core/Formatters.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/Pipeline.hpp"

#include <fmt/format.h>
#include <imgui_internal.h>

static constexpr float font_size = 11.0f;
static constexpr float frame_padding = 0.5f;

[[nodiscard]] static bool draw_vec3_control(const std::string& label, glm::vec3& values, float reset_value = 0.0f, float column_width = 100.0f)
{
	bool any_updated = false;
	ImGuiIO& io = ImGui::GetIO();
	auto bold_font = io.Fonts->Fonts[0];

	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, column_width);
	ImGui::Text("%s", label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 { 0, 0 });

	float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 button_size = { line_height + 3.0f, line_height };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushFont(bold_font);
	if (ImGui::Button("X", button_size)) {
		any_updated |= true;
		values.x = reset_value;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	any_updated |= ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushFont(bold_font);
	if (ImGui::Button("Y", button_size)) {
		any_updated |= true;
		values.y = reset_value;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	any_updated |= ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 { 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 { 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 { 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushFont(bold_font);
	if (ImGui::Button("Z", button_size)) {
		any_updated |= true;
		values.z = reset_value;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	any_updated |= ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	ImGui::Columns(1);

	ImGui::PopID();

	return any_updated;
}

namespace Disarray::Client {

template <ValidComponent T> void draw_component(Entity& entity, const std::string& name, auto&& ui_function)
{
	if (!entity.has_component<T>())
		return;

	const ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth
		| ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
	auto& component = entity.get_components<T>();
	ImVec2 content_region_available = ImGui::GetContentRegionAvail();

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2 { 4, 4 });
	float line_height = 10 + 2 + 1 * 2.0f;
	ImGui::Separator();

	const auto leaf_id = (const void*)typeid(T).hash_code();
	bool open = ImGui::TreeNodeEx(leaf_id, tree_node_flags, "%s", name.c_str());
	ImGui::PopStyleVar();
	ImGui::SameLine(content_region_available.x - line_height * 0.5f);
	if (ImGui::Button("+", ImVec2 { line_height, line_height })) {
		ImGui::OpenPopup("Component Settings");
	}

	bool remove_component = false;
	if (ImGui::BeginPopup("Component Settings")) {
		if (ImGui::MenuItem("Remove component"))
			remove_component = true;

		ImGui::EndPopup();
	}

	if (open) {
		ui_function(component);
		ImGui::TreePop();
	}

	if (remove_component)
		entity.remove_component<T>();
}

void ScenePanel::interface()
{
	auto& registry = scene.get_registry();

	UI::begin("Scene");
	std::unordered_set<entt::entity> selectable_entities;
	if (ImGui::BeginTable("Entities", 2)) {
		const auto inheritance_view = registry.view<const Components::ID, const Components::Tag, const Components::Inheritance>();
		for (const auto& [entity, id, tag, inheritance] : inheritance_view.each()) {
			selectable_entities.insert(entity);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("%s", tag.name.c_str());
			ImGui::TableNextColumn();
			const auto& children = inheritance.children;
			std::string text;
			if (!children.empty()) {
				text = fmt::format("{} - Children: [{}]", id.identifier, fmt::join(children, ", "));
			} else {
				text = fmt::format("{}", id.identifier);
			}
			ImGui::Text("%s", text.c_str());
		}

		const auto view = registry.view<const Components::ID, const Components::Tag>(entt::exclude<Components::Inheritance>);
		for (const auto& [entity, id, tag] : view.each()) {
			selectable_entities.insert(entity);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("%s", tag.name.c_str());
			ImGui::TableNextColumn();
			ImGui::Text("%u", id.get_id<std::uint32_t>());
		}

		ImGui::EndTable();
	}

	std::vector<Entity> entities;
	entities.reserve(selectable_entities.size());
	for (const auto& handle : selectable_entities) {
		entities.emplace_back(scene, handle);
	}

	UI::begin("Entity");
	for (auto& entity : entities) {
		if (!entity.has_component<Components::Pipeline>())
			continue;

		for_all_components(entity);
	}
	UI::end();

	UI::end();
} // namespace Disarray::Client

void Disarray::Client::ScenePanel::update(float) { }

void ScenePanel::for_all_components(Entity& entity)
{
	draw_component<Components::Transform>(entity, "Transform", [](Components::Transform& transform) {
		bool any_changed = false;

		any_changed |= draw_vec3_control("Position", std::ref(transform.position));
		auto euler_angles = eulerAngles(transform.rotation);
		any_changed |= draw_vec3_control("Rotation (Euler)", std::ref(euler_angles));
		any_changed |= draw_vec3_control("Scale", std::ref(transform.scale));

		if (any_changed) {
			transform.rotation = glm::quat(glm::vec3(euler_angles.x, euler_angles.y, euler_angles.z));
		}
	});

	draw_component<Components::Pipeline>(entity, "Pipeline", [&dev = device](Components::Pipeline& pipeline) {
		auto& [pipe] = pipeline;
		auto& props = pipe->get_properties();
		bool any_changed = false;
		any_changed |= UI::combo_choice<DepthCompareOperator>("Compare operator", std::ref(props.depth_comparison_operator));
		any_changed |= UI::combo_choice<CullMode>("Cull mode", std::ref(props.cull_mode));
		any_changed |= UI::combo_choice<FaceMode>("Face mode", std::ref(props.face_mode));
		any_changed |= UI::combo_choice<PolygonMode>("Polygon mode", std::ref(props.polygon_mode));
		any_changed |= UI::shader_drop_button(dev, "Vertex Shader", ShaderType::Vertex, std::ref(props.vertex_shader));
		any_changed |= UI::shader_drop_button(dev, "Fragment Shader", ShaderType::Fragment, std::ref(props.fragment_shader));
		if (any_changed) {
			pipe->recreate(true);
		}
	});
}

void Disarray::Client::ScenePanel::on_event(Event& event) { }

} // namespace Disarray::Client
