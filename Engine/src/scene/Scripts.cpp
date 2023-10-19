#include "scene/Scripts.hpp"

#include <core/Tuple.hpp>

#include <cstdint>

#include "graphics/RendererProperties.hpp"
#include "scene/Entity.hpp"
#include "ui/UI.hpp"

namespace Disarray::Scripts {

using namespace std::string_view_literals;

MoveInCircleScript::~MoveInCircleScript() = default;

MoveInCircleScript::MoveInCircleScript(std::uint32_t local_radius, float initial_angle, float velocity)
	: CppScript({ { "local_radius", local_radius }, { "angle", initial_angle }, { "velocity", velocity } })
	, radius(local_radius)
	, angle(glm::degrees(initial_angle))
	, vel(velocity)
{
}

MoveInCircleScript::MoveInCircleScript(const Collections::StringViewMap<Parameter>& params)
	: CppScript(params)
{
	const auto& local_radius = get_parameter_or("local_radius"sv, std::uint32_t { 10 });
	const auto& stored_angle = get_parameter_or("angle"sv, glm::radians(45.0F));
	const auto& stored_velocity = get_parameter_or("velocity"sv, 0.3F);

	radius = std::get<std::uint32_t>(local_radius);
	angle = glm::degrees(std::get<float>(stored_angle));
	vel = std::get<float>(stored_velocity);
}

void MoveInCircleScript::reload()
{
	const auto& local_radius = get_parameter_or("local_radius"sv, std::uint32_t { 10 });
	const auto& stored_angle = get_parameter_or("angle"sv, glm::radians(45.0F));
	const auto& stored_velocity = get_parameter_or("velocity"sv, 0.3F);

	radius = std::get<std::uint32_t>(local_radius);
	angle = glm::degrees(std::get<float>(stored_angle));
	vel = std::get<float>(stored_velocity);
}

void MoveInCircleScript::on_create() { }

void MoveInCircleScript::on_update(float time_step)
{
	auto& [rot, pos, scale] = transform();
	angle = angle + vel * time_step;
	rad = std::fmod(angle, 360);
	const auto radians = glm::radians(rad);
	pos.x = static_cast<float>(radius) * glm::sin(radians);
	pos.z = static_cast<float>(radius) * glm::cos(radians);
}

void MoveInCircleScript::on_interface() { }

LinearMovementScript::~LinearMovementScript() = default;

LinearMovementScript::LinearMovementScript(const Collections::StringViewMap<Parameter>& params)
	: CppScript(params)
{
	const auto& axis_as_uint8 = get_parameter_or("axis"sv, std::uint8_t { 0 });
	const auto& min_input = get_parameter_or("min"sv, -100.0F);
	const auto& max_input = get_parameter_or("max"sv, 100.0F);

	min = std::get<float>(min_input);
	max = std::get<float>(max_input);
	axis = static_cast<Axis>(std::get<std::uint8_t>(axis_as_uint8));
}

void LinearMovementScript::reload()
{
	const auto& axis_as_uint8 = get_parameter_or("axis"sv, std::uint8_t { 0 });
	const auto& min_input = get_parameter_or("min"sv, -100.0F);
	const auto& max_input = get_parameter_or("max"sv, 100.0F);

	min = std::get<float>(min_input);
	max = std::get<float>(max_input);
	axis = static_cast<Axis>(std::get<std::uint8_t>(axis_as_uint8));
}

LinearMovementScript::LinearMovementScript(float min_axis, float max_axis, Axis chosen)
	: CppScript({ { "min", min_axis }, { "max", max_axis }, { "axis", static_cast<std::uint8_t>(chosen) } })
	, axis(chosen)
	, min(min_axis)
	, max(max_axis)
{
	switch (axis) {
	case Axis::X:
		direction = { -1, 0, 0 };
		break;
	case Axis::Y:
		direction = { 0, -1, 0 };
		break;
	case Axis::Z:
		direction = { 0, 0, -1 };
		break;
	}
}

void LinearMovementScript::on_create() { }

void LinearMovementScript::on_update(float time_step)
{
	auto& [rot, pos, scale] = transform();
	pos = pos + vel * direction;
	switch (axis) {
	case Axis::X:
		if (pos.x <= min) {
			direction = glm::vec3 { 1, 0, 0 };
		}
		if (pos.x >= max) {
			direction = glm::vec3 { -1, 0, 0 };
		}
		break;
	case Axis::Y:
		if (pos.y >= max) {
			direction = glm::vec3 { 0, 1, 0 };
		}
		if (pos.y <= min) {
			direction = glm::vec3 { 0, -1, 0 };
		}
		break;
	case Axis::Z:
		if (pos.z >= max) {
			direction = glm::vec3 { 0, 0, 1 };
		}
		if (pos.z <= min) {
			direction = glm::vec3 { 0, 0, -1 };
		}
		break;
	default:
		return;
	}
}

void LinearMovementScript::on_interface() { }

void LinearMovementScript::on_render(Renderer& renderer)
{
	auto& [rot, pos, scale] = transform();

	const auto end = pos + direction * 2.F;
	renderer.draw_planar_geometry(Geometry::Line,
		{
			.position = pos,
			.to_position = end,
			.colour = { 0.6, 0.3, 0.9, 1.0 },
			.identifier = static_cast<std::uint32_t>(get_entity().get_identifier()),
		});
}

} // namespace Disarray::Scripts
