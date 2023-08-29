#include "scene/Scripts.hpp"

#include "ui/UI.hpp"

namespace Disarray::Scripts {

CustomScript::~CustomScript() {};

CustomScript::CustomScript(std::uint32_t local_radius, std::uint32_t count, float initial_angle)
	: radius(local_radius)
	, total_count(count)
	, angle(glm::degrees(initial_angle))
{
}

void CustomScript::on_create() { }

void CustomScript::on_update(float time_step)
{
	auto& [rot, pos, scale] = transform();
	angle = angle + vel * time_step;
	rad = std::fmod(angle, 360);
	const auto radians = glm::radians(rad);
	pos.x = radius * glm::sin(radians);
	pos.z = radius * glm::cos(radians);
}

void CustomScript::on_interface() { }

} // namespace Disarray::Scripts
