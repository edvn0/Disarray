#include "panels/ScenePanel.hpp"

#include <fmt/format.h>
#include <imgui_internal.h>

#include "core/Formatters.hpp"
#include "glm/common.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/dual_quaternion.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/Pipeline.hpp"
#include "scene/Camera.hpp"
#include "scene/Components.hpp"
#include "ui/InterfaceLayer.hpp"
#include "ui/UI.hpp"

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

	if (remove_component) {
		if constexpr (DeletableComponent<T>)
			entity.remove_component<T>();
	}
}

ScenePanel::ScenePanel(Device& dev, Window&, Swapchain&, Scene& s)
	: device(dev)
	, scene(s)
{

	selected_entity = std::make_unique<entt::entity>(entt::null);
}

void ScenePanel::draw_entity_node(Disarray::Entity& entity)
{
	const auto& tag = entity.get_components<Components::Tag>().name;

	const auto is_same = (*selected_entity == entity.get_identifier());
	ImGuiTreeNodeFlags flags = (is_same ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
	const auto& id = entity.get_components<Components::ID>();
	bool opened = ImGui::TreeNodeEx(Disarray::bit_cast<const void*>(id.identifier), flags, "%s", tag.c_str());
	if (ImGui::IsItemClicked()) {
		*selected_entity = entity.get_identifier();
	}

	bool entity_deleted = false;
	if (ImGui::BeginPopupContextWindow("##testtest")) {
		if (ImGui::MenuItem("Delete Entity"))
			entity_deleted = true;

		ImGui::EndPopup();
	}

	if (opened) {
		if (entity.has_component<Components::Inheritance>()) {
			const auto children = entity.get_components<Components::Inheritance>();
			for (const auto& child : children.children) {
				const auto child_entity = scene.get_by_identifier(child);
				if (!child_entity)
					continue;
				const auto& t = (*child_entity).get_components<Components::Tag>();
				ImGui::Text("%s", t.name.c_str());
			}
		}
		ImGui::TreePop();
	}

	if (entity_deleted) {
		scene.delete_entity(entity);
		if (*selected_entity == entity.get_identifier())
			selected_entity = {};
	}
}

void ScenePanel::interface()
{
	UI::begin("Scene");
	scene.for_all_entities([this](entt::entity entity_id) {
		Entity entity { scene, entity_id };
		draw_entity_node(entity);
	});

	if (ImGui::BeginPopupContextWindow("EmptyEntityId", 1)) {
		if (ImGui::MenuItem("Create Empty Entity"))
			scene.create("Empty Entity");

		ImGui::EndPopup();
	}
	UI::end();

	ImGui::Begin("Properties");
	if (!selected_entity) {
		UI::end();
		return;
	}

	if (auto ent = Entity(scene, *selected_entity); ent.is_valid())
		for_all_components(ent);
	UI::end();
} // namespace Disarray::Client

void Disarray::Client::ScenePanel::update(float, IGraphicsResource&)
{
	if (const auto& selected = scene.get_selected_entity(); selected && selected->is_valid()) {
		*selected_entity = selected->get_identifier();
	}
}

void ScenePanel::for_all_components(Entity& entity)
{
	draw_component<Components::Tag>(entity, "Tag", [](Components::Tag& tag) {
		std::string buffer = tag.name;
		buffer.resize(256);

		if (ImGui::InputText("Tag", buffer.data(), 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
			buffer.shrink_to_fit();

			if (!buffer.empty() && tag.name != buffer) {
				tag.name = buffer;
			}
		}
	});

	draw_component<Components::Transform>(entity, "Transform", [](Components::Transform& transform) {
		bool any_changed = false;

		any_changed |= ImGui::DragFloat3("Position", glm::value_ptr(transform.position));
		auto euler_angles = eulerAngles(transform.rotation);
		any_changed |= ImGui::DragFloat3("Rotation (Euler)", glm::value_ptr(euler_angles));
		any_changed |= ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale));

		if (any_changed) {
			transform.rotation = glm::quat(glm::vec3(euler_angles.x, euler_angles.y, euler_angles.z));
		}
	});

	draw_component<Components::Texture>(entity, "Texture", [&dev = device](Components::Texture& tex) {
		auto& [texture, colour] = tex;
		Ref<Texture> new_texture { nullptr };
		if (texture) {
			new_texture = UI::texture_drop_button(dev, *texture);
			ImGui::NewLine();
		}

		bool any_changed = false;
		any_changed |= new_texture != nullptr;

		ImGui::ColorEdit4("Colour", glm::value_ptr(colour));

		if (any_changed) {
			tex.texture.reset();
			tex.texture = new_texture;
		}
	});

	draw_component<Components::DirectionalLight>(entity, "Directional Light", [](Components::DirectionalLight& directional) {
		auto& [direction, intensity] = directional;

		if (glm::all(glm::isnan(direction)))
			direction = glm::vec3(0.0f);

		if (ImGui::DragFloat3("Direction", glm::value_ptr(direction))) { }
		if (ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.01f, 1.0f)) { }
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
			pipe->recreate(true, {});
		}
	});
}

void ScenePanel::on_event(Event& event) { }

} // namespace Disarray::Client
