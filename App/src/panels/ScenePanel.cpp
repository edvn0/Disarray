#include "panels/ScenePanel.hpp"

#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/dual_quaternion.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include "core/Formatters.hpp"
#include "graphics/CommandExecutor.hpp"
#include "graphics/Framebuffer.hpp"
#include "graphics/ImageProperties.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/Texture.hpp"
#include "scene/Camera.hpp"
#include "scene/Components.hpp"
#include "ui/InterfaceLayer.hpp"
#include "ui/UI.hpp"

namespace Disarray::Client {

static constexpr std::size_t string_buffer_size = 256;

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
		scene->update_picked_entity(entity.get_identifier());
	}

	bool entity_deleted = false;
	if (ImGui::BeginPopup("DeleteEntityPopup", ImGuiPopupFlags_MouseButtonRight)) {
		if (ImGui::MenuItem("Delete Entity")) {
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
	if (scene == nullptr) {
		UI::Scope empty("Scene");
		UI::text_wrapped("Currently empty scene!");
		return;
	};

	UI::begin("Scene");
	const auto& tag = scene->get_name();
	std::string buffer = tag;
	buffer.resize(string_buffer_size);

	if (ImGui::InputText("Scene name", buffer.data(), string_buffer_size, ImGuiInputTextFlags_EnterReturnsTrue)) {
		buffer.shrink_to_fit();

		if (!buffer.empty() && tag != buffer) {
			scene->set_name(buffer);
		}
	}

	scene->for_all_entities([this](entt::entity entity_id) {
		Entity entity { scene, entity_id };
		draw_entity_node(entity, true);
	});

	if (ImGui::BeginPopupContextWindow("EmptyEntityId", ImGuiPopupFlags_MouseButtonRight)) {
		if (ImGui::MenuItem("Create Empty Entity")) {
			scene->create("Empty Entity");
		}

		if (ImGui::MenuItem("Copy Entity")) {
			if (auto entity = Entity { scene, *selected_entity }; entity.is_valid()) {
				Scene::copy_entity(*scene, entity);
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

void Client::ScenePanel::update(float)
{
	if (scene == nullptr) {
		return;
	}

	if (const auto& selected = scene->get_selected_entity(); selected && selected->is_valid()) {
		*selected_entity = selected->get_identifier();
	}
}

// NOLINTBEGIN
void ScenePanel::for_all_components(Entity& entity)
{
	if (entity.has_component<Components::Tag>()) {
		auto& tag = entity.get_components<Components::Tag>();
		std::string buffer = tag.name;
		buffer.resize(string_buffer_size);

		if (ImGui::InputText("Tag", buffer.data(), string_buffer_size, ImGuiInputTextFlags_EnterReturnsTrue)) {
			buffer.shrink_to_fit();

			if (!buffer.empty() && tag.name != buffer) {
				tag.name = std::string { buffer.c_str() };
			}
		}
	}

	ImGui::SameLine();
	ImGui::PushItemWidth(-1);

	if (ImGui::Button("Add Component")) {
		ImGui::OpenPopup("AddComponent");
	}

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

	draw_component<Components::LineGeometry>(entity, [](Components::LineGeometry& line_geometry) {
		UI::text_wrapped("Line Geometry");
		std::ignore = ImGui::DragFloat3("To Position", glm::value_ptr(line_geometry.to_position));
	});
	draw_component<Components::QuadGeometry>(entity, [](Components::QuadGeometry& quad_geometry) { UI::text_wrapped("Quad Geometry"); });

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

	draw_component<Components::SpotLight>(entity, [](Components::SpotLight& spot) {
		if (ImGui::DragFloat3("Direction", glm::value_ptr(spot.direction), 0.1F, -glm::pi<float>(), glm::pi<float>())) { }
		if (UI::Input::drag("Factors", spot.factors, 0.1F, 0.F, 10.F)) { }
		if (ImGui::ColorEdit4("Ambient", glm::value_ptr(spot.ambient))) { }
		if (ImGui::ColorEdit4("Diffuse", glm::value_ptr(spot.diffuse))) { }
		if (ImGui::ColorEdit4("Specular", glm::value_ptr(spot.specular))) { }
		if (ImGui::DragFloat("Cutoff", &spot.cutoff_angle_degrees, 1.5F, 2.0F, 85.F)) { }
		if (ImGui::DragFloat("Outer Cutoff", &spot.outer_cutoff_angle_degrees, 1.5F, 2.0F, 85.F)) { }
		if (spot.cutoff_angle_degrees > spot.outer_cutoff_angle_degrees) {
			std::swap(spot.cutoff_angle_degrees, spot.outer_cutoff_angle_degrees);
		}
	});

	draw_component<Components::Text>(entity, [](Components::Text& text) {
		std::string buffer = text.text_data;
		buffer.resize(string_buffer_size);

		if (ImGui::InputText("Text", buffer.data(), string_buffer_size, ImGuiInputTextFlags_EnterReturnsTrue)) {
			buffer.shrink_to_fit();

			if (!buffer.empty() && text.text_data != buffer) {
				const auto* buffer_c_str = buffer.c_str();
				text.text_data = buffer_c_str;
			}
		}
		if (ImGui::ColorEdit4("Colour", glm::value_ptr(text.colour))) { }
		if (ImGui::DragFloat("Size", &text.size, 0.1F, 0.2F, 5.0F)) { }
		if (UI::combo_choice<Components::TextProjection>("Space Choice", std::ref(text.projection))) { }
	});

	draw_component<Components::Material>(entity, [&](Components::Material& mat) {
		if (!mat.material) {
			mat.material = Material::construct(device, {});
		}

		auto& properties = mat.material->get_properties();
		Collections::for_each_unwrapped(properties.textures, [](const auto& key, Ref<Disarray::Texture>& texture) {
			ImGui::PushID(key.c_str());
			UI::text("{}:", key);
			UI::image(*texture);
			ImGui::PopID();
		});

		ImGui::NewLine();
		std::optional<std::filesystem::path> selected { std::nullopt };
		if (ImGui::Button("Choose path", { 80, 30 })) {
			selected = UI::Popup::select_file(
				{
					"*.png",
					"*.bmp",
				},
				FS::texture_directory());
		}
		if (!selected.has_value()) {
			return;
		}
		const auto& path = *selected;

		properties.textures.try_emplace(path.filename().string(),
			Texture::construct(device,
				{
					.path = path,
					.debug_name = path.filename().string(),
				}));
	});

	draw_component<Components::Skybox>(entity, [&current = scene, &dev = device](Components::Skybox& skybox) {
		bool any_changed = false;
		std::optional<std::filesystem::path> selected { std::nullopt };
		if (ImGui::Button("Choose path", { 80, 30 })) {
			selected = UI::Popup::select_file({ "*.ktx", "*.png" }, "Assets/Textures");
			any_changed |= selected.has_value();
		}

		if (any_changed && selected.has_value()) {
			const auto& value = *selected;

			auto new_cubemap = Texture::construct(dev,
				{
					.path = value,
					.dimension = TextureDimension::Three,
					.debug_name = value.string(),
				});

			if (!new_cubemap->valid()) {
				return;
			}

			skybox.texture = std::move(new_cubemap);
			current->submit_preframe_work([](Scene& this_scene, SceneRenderer& renderer) {
				const auto& texture_cube = this_scene.get_by_components<Components::Skybox>()->get_components<Components::Skybox>().texture;

				auto& graphics_resource = renderer.get_graphics_resource();
				graphics_resource.expose_to_shaders(texture_cube->get_image(), DescriptorSet(2), DescriptorBinding(2));
			});
		}

		ImGui::SameLine();
		std::ignore = UI::button("Drop texture", { 80, 30 });
		if (const auto dropped = UI::accept_drag_drop("Disarray::DragDropItem",
				{
					".ktx",
				})) {
			const auto& texture_path = *dropped;
			skybox.texture = Texture::construct(dev,
				{
					.path = texture_path,
					.dimension = TextureDimension::Three,
					.debug_name = texture_path.string(),
				});
			current->submit_preframe_work([](Scene& this_scene, SceneRenderer& renderer) {
				const auto& texture_cube = this_scene.get_by_components<Components::Skybox>()->get_components<Components::Skybox>().texture;

				auto& graphics_resource = renderer.get_graphics_resource();
				graphics_resource.expose_to_shaders(texture_cube->get_image(), DescriptorSet(2), DescriptorBinding(2));
			});
		}
	});

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

	draw_component<Components::Mesh>(entity, [&dev = device](Components::Mesh& mesh_component) {
		auto& [path, _, flags, inputs] = mesh_component.mesh->get_properties();

		bool any_changed = false;
		std::optional<std::filesystem::path> selected { std::nullopt };
		if (ImGui::Button("Choose path", { 80, 30 })) {
			selected = UI::Popup::select_file({ "*.mesh", "*.obj", "*.fbx" }, "Assets/Models");
			any_changed |= selected.has_value();
		}
		if (ImGui::Checkbox("Draw AABB", &mesh_component.draw_aabb)) { };

		if (any_changed) {
			std::filesystem::path new_path {};
			if (selected.has_value()) {
				new_path = *selected;
			}
			if (mesh_component.mesh != nullptr) {
				path = new_path;
				mesh_component.mesh->force_recreation();

			} else {
				mesh_component.mesh = Mesh::construct(dev,
					{
						.path = new_path,
					});
			}
		}
	});

	draw_component<Components::Script>(entity, [](Components::Script& script_component) {
		UI::text("Script name: {}", script_component.get_script().identifier());
		auto& parameters = script_component.get_script().get_parameters();
		bool any_changed = false;
		for (auto&& [key, value] : parameters) {
			any_changed |= switch_parameter<glm::vec2>(value, [&key = key](glm::vec2& vector) { return UI::Input::slider(key, vector, 0.1F); });
			any_changed |= switch_parameter<glm::vec3>(value, [&key = key](glm::vec3& vector) { return UI::Input::slider(key, vector, 0.1F); });
			any_changed |= switch_parameter<glm::vec4>(value, [&key = key](glm::vec4& vector) { return UI::Input::slider(key, vector, 0.1F); });
			any_changed |= switch_parameter<std::uint32_t>(value, [&key = key](std::uint32_t& vector) {
				auto as_int = static_cast<std::int32_t>(vector);
				const bool changed = ImGui::DragInt(key.data(), &as_int);
				if (changed) {
					vector = static_cast<std::uint32_t>(as_int);
				}
				return changed;
			});
			any_changed |= switch_parameter<std::uint8_t>(value, [&key = key](std::uint8_t& vector) {
				auto as_int = static_cast<std::int32_t>(vector);
				const bool changed = ImGui::DragInt(key.data(), &as_int);
				if (changed) {
					vector = static_cast<std::uint8_t>(as_int);
				}
				return changed;
			});
			any_changed |= switch_parameter<float>(value, [&key = key](float& vector) { return UI::Input::slider<1>(key, &vector, 0.1F); });
			any_changed |= switch_parameter<double>(value, [&key = key](double& vector) {
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

	draw_component<Components::CapsuleCollider>(entity, [&](Components::CapsuleCollider& collider) {
		if (ImGui::DragFloat("Radius", &collider.radius)) { }
		if (ImGui::DragFloat("Height", &collider.height)) { }
		if (ImGui::DragFloat3("Offset", glm::value_ptr(collider.offset))) { }
	});

	draw_component<Components::ColliderMaterial>(entity, [&](Components::ColliderMaterial& material) {
		if (ImGui::DragFloat("Bounciness", &material.bounciness, 0.05F, 0.0F, 1.0F)) { }
		if (ImGui::DragFloat("Mass Density", &material.mass_density, 0.05F, 0.0F, 1.0F)) { }
		if (ImGui::DragFloat("Friction coefficient", &material.friction_coefficient, 0.05F, 0.0F, 1.0F)) { }
	});

	draw_component<Components::BoxCollider>(entity, [&](Components::BoxCollider& collider) {
		if (ImGui::DragFloat3("Offset", glm::value_ptr(collider.offset))) { }
		if (ImGui::DragFloat3("Half Extents", glm::value_ptr(collider.half_size))) { }
	});

	draw_component<Components::SphereCollider>(entity, [&](Components::SphereCollider& collider) {
		if (ImGui::DragFloat("Radius", &collider.radius)) { }
		if (ImGui::DragFloat3("Offset", glm::value_ptr(collider.offset))) { }
	});

	draw_component<Components::RigidBody>(entity, [&](Components::RigidBody& body) {
		if (UI::combo_choice<BodyType>("Body Type", std::ref(body.body_type))) { }
		if (ImGui::DragFloat("Mass", &body.mass)) { }
		if (ImGui::DragFloat("Linear Drag", &body.linear_drag)) { }
		if (ImGui::DragFloat("Angular Drag", &body.angular_drag)) { }
		if (UI::checkbox("Disable gravity", body.disable_gravity)) { }
		if (UI::checkbox("Kinematic", body.is_kinematic)) { }
	});
}
// NOLINTEND

void ScenePanel::on_event(Event& event) { }

} // namespace Disarray::Client
