#include "panels/ScenePanel.hpp"

#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/dual_quaternion.hpp>

#include <fmt/format.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "core/Formatters.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/Pipeline.hpp"
#include "scene/Camera.hpp"
#include "scene/Components.hpp"
#include "ui/InterfaceLayer.hpp"
#include "ui/UI.hpp"

namespace Disarray::Client {

ScenePanel::ScenePanel(Device& dev, Window&, Swapchain&, Scene* s)
	: device(dev)
	, scene(s)
{

	selected_entity = std::make_unique<entt::entity>(entt::null);
}

void ScenePanel::draw_entity_node(Disarray::Entity& entity, bool check_if_has_parent, std::uint32_t depth)
{
	static constexpr auto max_recursion = 4;
	const auto has_inheritance = entity.has_component<Components::Inheritance>();
	const auto has_children = has_inheritance && entity.get_components<Components::Inheritance>().has_children();
	const auto has_hit_max_recursion = depth >= max_recursion;
	if (!has_hit_max_recursion && check_if_has_parent && has_inheritance) {
		const auto& inheritance = entity.get_components<Components::Inheritance>();
		if (inheritance.has_parent()) {
			return;
		}
	}
	if (has_hit_max_recursion) {
		UI::text_wrapped("There are more children, but we only support {} children levels for now.", max_recursion);
		return;
	}

	const auto& tag = entity.get_components<Components::Tag>().name;

	const auto is_same = selected_entity && (*selected_entity == entity.get_identifier());
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | (is_same ? ImGuiTreeNodeFlags_Selected : 0);
	if (!has_children) {
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
	const auto& id_component = entity.get_components<Components::ID>();
	bool opened = ImGui::TreeNodeEx(Disarray::bit_cast<const void*>(&id_component.identifier), flags, "%s", tag.c_str());
	if (ImGui::IsItemClicked()) {
		*selected_entity = entity.get_identifier();
	}

	bool entity_deleted = false;
	if (ImGui::BeginPopupContextWindow("DeleteEntityPopup", ImGuiPopupFlags_MouseButtonRight)) {
		if (ImGui::MenuItem("Delete Entity")) {
			entity_deleted = true;
		}
		if (ImGui::MenuItem("Add Component")) {
			entity_deleted = true;
		}
		ImGui::EndPopup();
	}

	if (opened) {
		if (!has_inheritance) {
			ImGui::TreePop();
			return;
		}

		const auto& children = entity.get_components<Components::Inheritance>();
		for (const auto& child : children.children) {
			auto child_entity = scene->get_by_identifier(child);
			if (!child_entity) {
				continue;
			}
			draw_entity_node(*child_entity, false, depth + 1);
		}
		ImGui::TreePop();
	}

	if (entity_deleted) {
		scene->delete_entity(entity);
		if (*selected_entity == entity.get_identifier()) {
			*selected_entity = {};
		}
	}
}

void ScenePanel::interface()
{
	UI::begin("Scene");
	scene->for_all_entities([this](entt::entity entity_id) {
		Entity entity { scene, entity_id };
		draw_entity_node(entity, true);
	});

	if (ImGui::BeginPopupContextWindow("EmptyEntityId", ImGuiPopupFlags_MouseButtonRight)) {
		if (ImGui::MenuItem("Create Empty Entity")) {
			if (auto entity = Entity { scene, *selected_entity }; entity.is_valid()) {
				auto child = scene->create("Parent{}-Child", static_cast<Identifier>(entity.get_identifier()));
				entity.add_child(child);
			} else {
				scene->create("Empty Entity");
			}
		}

		ImGui::EndPopup();
	}
	UI::end();

	ImGui::Begin("Properties");
	if (!selected_entity) {
		UI::end();
		return;
	}

	if (auto ent = Entity(scene, *selected_entity); ent.is_valid()) {
		for_all_components(ent);
	}
	UI::end();
} // namespace Disarray::Client

void Disarray::Client::ScenePanel::update(float)
{
	if (const auto& selected = scene->get_selected_entity(); selected && selected->is_valid()) {
		*selected_entity = selected->get_identifier();
	}
}

void ScenePanel::for_all_components(Entity& entity)
{
	if (entity.has_component<Components::Tag>()) {
		auto& tag = entity.get_components<Components::Tag>();
		std::string buffer = tag.name;
		buffer.resize(256);

		if (ImGui::InputText("Tag", buffer.data(), 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
			buffer.shrink_to_fit();

			if (!buffer.empty() && tag.name != buffer) {
				tag.name = buffer;
			}
		}
	}

	ImGui::SameLine();
	ImGui::PushItemWidth(-1);

	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("AddComponent");

	if (ImGui::BeginPopup("AddComponent")) {
		draw_add_component_all(AllComponents {});

		ImGui::EndPopup();
	}

	ImGui::PopItemWidth();

	draw_component<Components::Transform>(entity, [](Components::Transform& transform) {
		bool any_changed = false;

		any_changed |= ImGui::DragFloat3("Position", glm::value_ptr(transform.position));
		auto euler_angles = glm::degrees(eulerAngles(transform.rotation));
		any_changed |= ImGui::DragFloat3("Rotation (Euler)", glm::value_ptr(euler_angles), 2.F, -180, 180);
		any_changed |= ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale));

		if (any_changed) {
			transform.rotation = glm::quat(glm::radians(euler_angles));
		}
	});

	draw_component<Components::Texture>(entity, [&dev = device](Components::Texture& tex) {
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

	draw_component<Components::DirectionalLight>(entity, [](Components::DirectionalLight& directional) {
		if (ImGui::ColorEdit4("Ambient", glm::value_ptr(directional.ambient))) { }
		if (ImGui::ColorEdit4("Diffuse", glm::value_ptr(directional.diffuse))) { }
		if (ImGui::ColorEdit4("Specular", glm::value_ptr(directional.specular))) { }
		if (ImGui::DragFloat3("Direction", glm::value_ptr(directional.direction), 0.1F, -glm::pi<float>(), glm::pi<float>())) { }
		if (ImGui::DragFloat("Factor", &directional.projection_parameters.factor)) { }
		if (ImGui::DragFloat("Near", &directional.projection_parameters.near)) { }
		if (ImGui::DragFloat("Far", &directional.projection_parameters.far)) { }
		if (ImGui::DragFloat("Fov", &directional.projection_parameters.fov, 2.F, 0.5F, 180.F)) { }

		if (ImGui::Checkbox("Direction Vector", &directional.use_direction_vector)) { }
	});

	draw_component<Components::PointLight>(entity, [](Components::PointLight& point) {
		if (UI::Input::drag("Factors", point.factors, 0.1F, 0.F, 10.F)) { }
		if (ImGui::ColorEdit4("Ambient", glm::value_ptr(point.ambient))) { }
		if (ImGui::ColorEdit4("Diffuse", glm::value_ptr(point.diffuse))) { }
		if (ImGui::ColorEdit4("Specular", glm::value_ptr(point.specular))) { }
	});

	draw_component<Components::Text>(entity, [](Components::Text& text) {
		std::string buffer = text.text_data;
		buffer.resize(256);

		if (ImGui::InputText("Text", buffer.data(), 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
			buffer.shrink_to_fit();

			if (!buffer.empty() && text.text_data != buffer) {
				text.text_data = buffer;
			}
		}
		if (ImGui::ColorEdit4("Colour", glm::value_ptr(text.colour))) { }
		if (ImGui::DragFloat("Size", &text.size, 0.1F, 0.2F, 5.0F)) { }
		if (UI::combo_choice<Components::TextProjection>("Space Choice", std::ref(text.projection))) { }
	});

	draw_component<Components::Material>(entity, [](Components::Material& mat) {
		const auto& props = mat.material->get_properties();
		UI::text_wrapped("VS: {}", props.vertex_shader->get_properties().identifier.string());
		UI::text_wrapped("FS: {}", props.fragment_shader->get_properties().identifier.string());
		for (const auto& text : props.textures) {
			UI::text("{}", text->get_properties().debug_name);
			UI::image(text->get_image());
		}
	});

	draw_component<Components::Skybox>(entity, [](Components::Skybox& skybox) { UI::image(skybox.texture->get_image()); });

	draw_component<Components::Camera>(entity, [](Components::Camera& cam) {
		std::ignore = UI::combo_choice<CameraType>("Type", std::ref(cam.type));

		if (cam.type == CameraType::Perspective) {
			if (ImGui::DragFloat("Near", &cam.near_perspective)) { }
			if (ImGui::DragFloat("Far", &cam.far_perspective)) { }
		} else {
			if (ImGui::DragFloat("Near", &cam.near_orthographic)) { }
			if (ImGui::DragFloat("Far", &cam.far_orthographic)) { }
		}
		if (ImGui::DragFloat("Fov", &cam.fov_degrees, 2.F, 4.F, 160.F)) { }
		if (ImGui::Checkbox("Primary", &cam.is_primary)) { }
		if (ImGui::Checkbox("Reverse", &cam.reverse)) { }
	});

	draw_component<Components::Mesh>(entity, [](Components::Mesh& mesh_component) {
		auto& [path, pipeline, _, flags, inputs] = mesh_component.mesh->get_properties();

		bool any_changed = false;
		std::optional<std::filesystem::path> selected { std::nullopt };
		if (ImGui::Button("Choose path", { 80, 30 })) {
			selected = UI::Popup::select_file({ "*.mesh" });
			any_changed |= selected.has_value();
		}
		if (ImGui::Checkbox("Draw AABB", &mesh_component.draw_aabb)) { };

		if (any_changed) {
			if (selected.has_value()) {
				path = *selected;
			}
			mesh_component.mesh->force_recreation();
		}
	});

	draw_component<Components::Script>(entity, [](Components::Script& script_component) {
		UI::text("Script name: {}", script_component.get_script().identifier());
		auto& parameters = script_component.get_script().get_parameters();
		bool any_changed = false;
		for (auto&& [key, value] : parameters) {
			any_changed |= switch_parameter<glm::vec2>(value, [key](glm::vec2& vector) { return UI::Input::slider(key, vector, 0.1F); });
			any_changed |= switch_parameter<glm::vec3>(value, [key](glm::vec3& vector) { return UI::Input::slider(key, vector, 0.1F); });
			any_changed |= switch_parameter<glm::vec4>(value, [key](glm::vec4& vector) { return UI::Input::slider(key, vector, 0.1F); });
			any_changed |= switch_parameter<float>(value, [key](float& vector) { return UI::Input::slider<1>(key, &vector, 0.1F); });
			any_changed |= switch_parameter<double>(value, [key](double& vector) {
				auto as_float = static_cast<float>(vector);
				bool changed = ImGui::DragFloat(key.data(), &as_float, 0.1F);
				if (changed) {
					vector = static_cast<double>(as_float);
				}
				return changed;
			});
		}

		if (any_changed) {
			script_component.get_script().reload();
		}
	});

	draw_component<Components::Pipeline>(entity, [&dev = device](Components::Pipeline& pipeline) {
		auto& [pipe, identifier] = pipeline;
		auto& props = pipe->get_properties();
		bool any_changed = false;
		any_changed |= UI::combo_choice<DepthCompareOperator>("Compare operator", std::ref(props.depth_comparison_operator));
		any_changed |= UI::combo_choice<CullMode>("Cull mode", std::ref(props.cull_mode));
		any_changed |= UI::combo_choice<FaceMode>("Face mode", std::ref(props.face_mode));
		any_changed |= UI::combo_choice<PolygonMode>("Polygon mode", std::ref(props.polygon_mode));
		any_changed |= UI::shader_drop_button(dev, "Vertex Shader", ShaderType::Vertex, std::ref(props.vertex_shader));
		any_changed |= UI::shader_drop_button(dev, "Fragment Shader", ShaderType::Fragment, std::ref(props.fragment_shader));
		any_changed |= UI::checkbox("Depth test", props.test_depth);
		any_changed |= UI::checkbox("Depth write", props.write_depth);

		if (any_changed) {
			pipe->recreate(true, {});
		}
	});

	draw_component<Components::PillCollider>(entity, [&](Components::PillCollider& collider) {

	});

	draw_component<Components::BoxCollider>(entity, [&](Components::BoxCollider collider) {

	});

	draw_component<Components::SphereCollider>(entity, [&](Components::SphereCollider& collider) {

	});
}

void ScenePanel::on_event(Event& event) { }

} // namespace Disarray::Client
