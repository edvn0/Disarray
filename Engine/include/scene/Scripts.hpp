#pragma once

#include <glm/glm.hpp>

#include "core/Log.hpp"
#include "scene/CppScript.hpp"

namespace Disarray::Scripts {

class CustomScript final : public CppScript {
public:
	~CustomScript() override;
	CustomScript(std::uint32_t local_radius, std::uint32_t count, float initial_angle);

	void on_create() override;
	void on_interface() override;
	void on_update(float time_step) override;

private:
	[[nodiscard]] auto script_name() const -> std::string_view override { return "MoveInCircle"; }
	std::uint32_t radius {};
	float angle {};
	float vel { 0.03F };
	float rad { glm::radians(1.5F) };
};

} // namespace Disarray::Scripts
