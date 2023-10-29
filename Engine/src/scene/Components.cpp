#include "DisarrayPCH.hpp"

#include "scene/Components.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/quaternion_transform.hpp>

#include <graphics/Maths.hpp>

#include <string>
#include <utility>

#include "core/Formatters.hpp"
#include "core/Input.hpp"
#include "core/Log.hpp"
#include "graphics/Mesh.hpp"
#include "scene/Camera.hpp"
#include "scene/CppScript.hpp"
#include "scene/Entity.hpp"
#include "scene/Scene.hpp"

namespace Disarray::Components {

void Inheritance::add_child(Entity& entity) { children.insert(entity.get_components<Components::ID>().identifier); }

Mesh::Mesh(Device& device, std::string_view path)
	: mesh(Disarray::Mesh::construct(device,
		MeshProperties {
			.path = std::string { path },
		}))
{
}

Mesh::Mesh(Ref<Disarray::Mesh> input_mesh)
	: mesh(std::move(input_mesh))
{
}

Material::Material(Device& device, std::string_view vertex, std::string_view fragment)
	: material(Disarray::Material::construct(device,
		MaterialProperties {
			.vertex_shader = Shader::construct(device,
				{
					.path = std::filesystem::path { vertex },
					.identifier = vertex,
				}),
			.fragment_shader = Shader::construct(device,
				{
					.path = std::filesystem::path { fragment },
					.identifier = fragment,
				}),
		}))
{
}

Material::Material(Ref<Disarray::Material> m)
	: material(std::move(m))
{
}

Pipeline::Pipeline(Ref<Disarray::Pipeline> p)
	: pipeline(std::move(p))
{
}

Texture::Texture(Device& device, std::string_view path)
	: texture(Disarray::Texture::construct(device,
		TextureProperties {
			.path = std::string { path },
			.debug_name = std::string { path },
		}))
{
}

Texture::Texture(Ref<Disarray::Texture> m, const glm::vec4& colour)
	: texture(std::move(m))
	, colour(colour)
{
}

Texture::Texture(const glm::vec4& colour)
	: colour(colour)
{
}

void Script::Deleter::operator()(CppScript* script) noexcept { delete script; }

void Script::setup_entity_destruction()
{
	destroy_script_functor = [](Script& script) {
		script.instance_slot->on_destroy();
		script.instance_slot.reset();
	};
}

void Script::setup_entity_creation() { get_script().on_create(); }

void Script::destroy() { destroy_script_functor(*this); }

void Script::instantiate() { instantiated = true; }

auto Script::get_script() -> CppScript& { return *instance_slot; }

[[nodiscard]] auto Script::get_script() const -> const CppScript& { return *instance_slot; }

[[nodiscard]] auto Script::has_been_bound() const -> bool { return bound && !instantiated; }

auto DirectionalLight::ProjectionParameters::compute() const -> glm::mat4
{
	const auto left = factor * -1;
	const auto right = factor;
	const auto bottom = left;
	const auto top = right;
	return Maths::ortho(left, right, bottom, top, near, far);
}

auto Controller::on_update(float time_step, Transform& transform) -> void { }

auto Camera::compute(const Disarray::Components::Transform& transform, const Disarray::Extent& extent) const
	-> const std::tuple<glm::mat4, glm::mat4, glm::mat4>&
{
	static auto pre_computed_for_parameters = std::unordered_map<std::size_t, std::tuple<glm::mat4, glm::mat4, glm::mat4>> {};
	static auto hash_this = [](auto... to_combine) {
		std::size_t seed { 0 };
		hash_combine(seed, to_combine...);
		return seed;
	};

	const auto hash = hash_this(
		fov_degrees, static_cast<std::uint8_t>(type), transform.position, extent.width, extent.height, near_perspective, far_perspective, reverse);

	bool should_skip = false;
	if (Input::all<KeyCode::R, KeyCode::LeftShift>()) {
		should_skip = true;
	}

	const auto did_contain = pre_computed_for_parameters.contains(hash);
	if (!should_skip && did_contain) {
		return pre_computed_for_parameters.at(hash);
	}

	auto view = glm::lookAt(transform.position, { 0, 0, 0 }, { 0, 1, 0 });
	const auto as_float = extent.as<float>();
	glm::mat4 projection;
	if (type == CameraType::Perspective) {
		projection = glm::perspectiveFov(glm::radians(fov_degrees), as_float.width, as_float.height, reverse ? far_perspective : near_perspective,
			reverse ? near_perspective : far_perspective);
	} else {
		const auto aspect = as_float.aspect_ratio();
		projection = glm::ortho<float>(
			-aspect, aspect, -1, 1, reverse ? far_orthographic : near_orthographic, reverse ? near_orthographic : far_orthographic);
	}

	if (should_skip && did_contain) {
		pre_computed_for_parameters[hash] = { view, projection, projection * view };
	} else {
		pre_computed_for_parameters.try_emplace(hash, view, projection, projection * view);
	}
	return pre_computed_for_parameters.at(hash);
}

Skybox::Skybox(Ref<Disarray::Texture> tex, const glm::vec4& col)
	: texture(tex)
	, colour(col)
{
}

DirectionalLight::DirectionalLight(const glm::vec4& ambience, ProjectionParameters params)
	: projection_parameters(params)
	, ambient(Maths::scale_colour(ambience))
{
	Log::info("Components", "Ambience: {}", ambient);
}

} // namespace Disarray::Components
