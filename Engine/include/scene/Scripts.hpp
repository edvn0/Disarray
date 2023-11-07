#pragma once

#include <glm/glm.hpp>

#include <concepts>

#include "core/Collections.hpp"
#include "scene/CppScript.hpp"

namespace Disarray::Scripts {

class MoveInCircleScript final : public CppScript {
	static constexpr auto default_velocity = 0.03F;

public:
	~MoveInCircleScript() override;
	MoveInCircleScript(std::integral auto local_radius, std::floating_point auto initial_angle, float velocity = default_velocity)
		: MoveInCircleScript(static_cast<std::uint32_t>(local_radius), static_cast<float>(initial_angle), velocity)
	{
	}

	MoveInCircleScript(std::uint32_t local_radius, float initial_angle, float velocity);
	MoveInCircleScript(const Collections::StringViewMap<Parameter>& parameters);

	void on_create() override;
	void on_interface() override;
	void on_update(float time_step) override;
	void reload() override;

private:
	[[nodiscard]] auto script_name() const -> std::string_view override { return "MoveInCircle"; }
	std::uint32_t radius {};
	float angle {};
	float vel { default_velocity };
	float rad { glm::radians(1.5F) };
};

class LinearMovementScript final : public CppScript {
public:
	enum class Axis : std::uint8_t { X, Y, Z };
	~LinearMovementScript() override;
	LinearMovementScript(float min, float max, Axis);
	LinearMovementScript(float min, float max)
		: LinearMovementScript(min, max, Axis::X) {};

	LinearMovementScript(std::integral auto min, std::integral auto max)
		: LinearMovementScript(static_cast<float>(min), static_cast<float>(max), Axis::X) {};
	LinearMovementScript(std::integral auto min, std::integral auto max, Axis axis)
		: LinearMovementScript(static_cast<float>(min), static_cast<float>(max), axis) {};
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
