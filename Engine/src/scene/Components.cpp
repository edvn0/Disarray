#include "DisarrayPCH.hpp"

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
#include "scene/Components.hpp"
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

Texture::Texture(Device& device, std::string_view path)
	: texture(Disarray::Texture::construct(device,
		TextureProperties {
			.path = std::string { path },
			.debug_name = std::string { path },
		}))
{
}

Texture::Texture(Ref<Disarray::Texture> input, const glm::vec4& colour)
	: texture(std::move(input))
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
	-> std::tuple<glm::mat4, glm::mat4, glm::mat4>
{
	const auto rotation = glm::mat4_cast(transform.rotation);
	const auto position = rotation * glm::vec4 { transform.position, 1.0F };

	auto view = glm::lookAt(glm::vec3(position), { 0, 0, 0 }, { 0, 1, 0 });
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

	return { view, projection, view * projection };
}

Skybox::Skybox(Ref<Disarray::Texture> tex, const glm::vec4& col)
	: texture(std::move(tex))
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
