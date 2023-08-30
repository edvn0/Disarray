#include "scene/Scripts.hpp"

#include "ui/UI.hpp"

namespace Disarray::Scripts {

MoveInCircleScript::~MoveInCircleScript() = default;

MoveInCircleScript::MoveInCircleScript(std::uint32_t local_radius, std::uint32_t, float initial_angle)
	: radius(local_radius)
	, angle(glm::degrees(initial_angle))
{
}

void MoveInCircleScript::on_create() { }

void MoveInCircleScript::on_update(float time_step)
{
	auto& [rot, pos, scale] = transform();
	angle = angle + vel * time_step;
	rad = std::fmod(angle, 360);
	const auto radians = glm::radians(rad);
	pos.x = radius * glm::sin(radians);
	pos.z = radius * glm::cos(radians);
}

void MoveInCircleScript::on_interface() { }

LinearMovementScript::~LinearMovementScript() = default;

LinearMovementScript::LinearMovementScript(float min_axis, float max_axis, Axis chosen)
	: axis(chosen)
	, min(min_axis)
	, max(max_axis)
{
}

void LinearMovementScript::on_create() { }

void LinearMovementScript::on_update(float time_step)
{
	auto& [rot, pos, scale] = transform();
	switch (axis) {
	case Axis::X:
		pos.x += direction * vel * time_step;
		if (pos.x >= max)
			direction = -1;
		if (pos.x <= min)
			direction = 1;
		break;
	case Axis::Y:
		pos.y += direction * vel * time_step;
		if (pos.x >= max)
			direction = -1;
		if (pos.x <= min)
			direction = 1;
		break;
	case Axis::Z:
		pos.z += direction * vel * time_step;
		if (pos.x >= max)
			direction = -1;
		if (pos.x <= min)
			direction = 1;
		break;
	default:
		return;
	}
}

void LinearMovementScript::on_interface() { }

} // namespace Disarray::Scripts
