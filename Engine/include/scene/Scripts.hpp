#pragma once

#include <glm/glm.hpp>

#include "core/Collections.hpp"
#include "scene/CppScript.hpp"

namespace Disarray::Scripts {

class MoveInCircleScript final : public CppScript {
public:
	~MoveInCircleScript() override;
	MoveInCircleScript(std::uint32_t local_radius, std::uint32_t count, float initial_angle);
	MoveInCircleScript(const Collections::StringViewMap<Parameter>& parameters);

	void on_create() override;
	void on_interface() override;
	void on_update(float time_step) override;
	void reload() override;

private:
	[[nodiscard]] auto script_name() const -> std::string_view override { return "MoveInCircle"; }
	std::uint32_t radius {};
	float angle {};
	float vel { 0.03F };
	float rad { glm::radians(1.5F) };
};

class LinearMovementScript final : public CppScript {
public:
	enum class Axis : std::uint8_t { X, Y, Z };
	~LinearMovementScript() override;
	LinearMovementScript(float min, float max, Axis);
	LinearMovementScript(float min, float max)
		: LinearMovementScript(min, max, Axis::X) {};
	LinearMovementScript(const Collections::StringViewMap<Parameter>& parameters);

	void on_create() override;
	void on_interface() override;
	void on_render(Disarray::Renderer&) override;
	void on_update(float time_step) override;
	void reload() override;

private:
	[[nodiscard]] auto script_name() const -> std::string_view override { return "LinearMovement"; }
	float vel { 0.03F };
	Axis axis { Axis::X };
	float min {};
	float max {};
	glm::vec3 direction { 0, 0, 0 };
};

} // namespace Disarray::Scripts
