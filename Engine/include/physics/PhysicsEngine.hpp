#pragma once

#include <glm/fwd.hpp>

#include "core/PointerDefinition.hpp"
#include "physics/PhysicsProperties.hpp"

namespace Disarray {

struct RigidBodyParameters {
	glm::vec3 position;
	glm::quat rotation;
	float linear_drag;
	float angular_drag;
	BodyType type;

	float mass { 1.0F };
	bool disable_gravity { false };
	bool is_kinematic { false };
};

struct CollisionMaterial {
	float friction_coefficient { 0.2F };
	float bounciness { 0.2F };
	float mass_density { 1.0F };
};

struct ColliderParameters {
	float radius { 1.0F };
	float height { 1.0F };
	glm::vec3 half_size { 0.5F };
	glm::vec3 offset { 0.0F };
	bool is_trigger { false };

	CollisionMaterial collision_material {};
};

using EngineRigidBody = void*;

struct EngineImpl;

class PhysicsEngine {
public:
	PhysicsEngine(std::uint32_t velocity_its, std::uint32_t position_its);

	auto step(float time_step) -> void;
	auto step(float time_step, std::uint32_t velocity_its, std::uint32_t positions_its) -> void;
	auto restart() -> void;

	/**
	 * @brief Create a rigid body object
	 *
	 * @return void* A pointer to the underlying engine's rigidbody.
	 */
	auto create_rigid_body(const RigidBodyParameters&) -> void*;

	auto add_capsule_collider(EngineRigidBody rigid_body, const ColliderParameters&) -> void;
	auto add_box_collider(EngineRigidBody rigid_body, const ColliderParameters&) -> void;
	auto add_sphere_collider(EngineRigidBody rigid_body, const ColliderParameters&) -> void;

	auto get_transform(EngineRigidBody) -> std::tuple<glm::vec3, glm::quat>;

private:
	std::uint32_t velocity_iterations {};
	std::uint32_t position_iterations {};

	Scope<EngineImpl, PimplDeleter<EngineImpl>> engine;
};

} // namespace Disarray
