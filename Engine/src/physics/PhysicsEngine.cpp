#include "DisarrayPCH.hpp"

#include <reactphysics3d/reactphysics3d.h>

#include "core/PointerDefinition.hpp"
#include "core/Types.hpp"
#include "physics/PhysicsEngine.hpp"
#include "reactphysics3d/body/RigidBody.h"
#include "reactphysics3d/collision/shapes/BoxShape.h"
#include "reactphysics3d/collision/shapes/SphereShape.h"
#include "reactphysics3d/components/RigidBodyComponents.h"
#include "scene/Components.hpp"

namespace Disarray {

namespace {
	template <class Vector>
		requires(std::is_same_v<Vector, glm::vec2> || std::is_same_v<Vector, glm::vec3>)
	constexpr auto to_react_from_glm(const Vector& vector)
	{
		if constexpr (std::is_same_v<Vector, glm::vec2>) {
			return rp3d::Vector2(vector[0], vector[1]);
		}
		if constexpr (std::is_same_v<Vector, glm::vec3>) {
			return rp3d::Vector3(vector[0], vector[1], vector[2]);
		}
	}

	auto to_react_from_glm(const glm::quat& quaternion)
	{
		const auto rotation_matrix = glm::mat4_cast(quaternion);
		const auto rot3d = glm::mat3(rotation_matrix);
		return rp3d::Matrix3x3(rot3d[0][0], rot3d[0][1], rot3d[0][2], rot3d[1][0], rot3d[1][1], rot3d[1][2], rot3d[2][0], rot3d[2][1], rot3d[2][2]);
	}

	template <class Parameters>
		requires requires(Parameters p) {
			{
				p.position
			} -> std::convertible_to<glm::vec3>;
			{
				p.rotation
			} -> std::convertible_to<glm::quat>;
		}
	constexpr auto to_react_transform_from_parameters(const auto& parameters)
	{
		return rp3d::Transform(to_react_from_glm(parameters.position), to_react_from_glm(parameters.rotation));
	}

	auto to_react_body_type_from_body_type(const BodyType& type)
	{
		switch (type) {
		case BodyType::Static:
			return rp3d::BodyType::STATIC;
		case BodyType::Dynamic:
			return rp3d::BodyType::DYNAMIC;
		default:
			unreachable();
		}
	}

} // namespace

struct PhysicsEngine::EngineImpl {
	reactphysics3d::PhysicsCommon common;
	reactphysics3d::PhysicsWorld* world { nullptr };
};

template <> auto PimplDeleter<PhysicsEngine::EngineImpl>::operator()(PhysicsEngine::EngineImpl* ptr) noexcept -> void { delete ptr; }

PhysicsEngine::PhysicsEngine(std::uint32_t velocity_its, std::uint32_t position_its)
	: velocity_iterations(velocity_its)
	, position_iterations(position_its)
{
	restart();
}

auto PhysicsEngine::restart() -> void
{
	engine.reset();
	engine = make_scope<PhysicsEngine::EngineImpl, PimplDeleter<PhysicsEngine::EngineImpl>>();

	engine->world = engine->common.createPhysicsWorld();
}

auto PhysicsEngine::step(float time_step) -> void { engine->world->update(time_step); }

auto PhysicsEngine::step(float time_step, std::uint32_t velocity_its, std::uint32_t position_its) -> void
{
	engine->world->setNbIterationsVelocitySolver(velocity_its);
	engine->world->setNbIterationsPositionSolver(position_its);
	engine->world->update(time_step);
}

auto PhysicsEngine::create_rigid_body(const RigidBodyParameters& parameters) -> void*
{
	auto transform = to_react_transform_from_parameters<RigidBodyParameters>(parameters);
	auto* rigid_body = engine->world->createRigidBody(transform);
	rigid_body->setType(to_react_body_type_from_body_type(parameters.type));
	rigid_body->setMass(parameters.mass);
	rigid_body->setAngularDamping(parameters.angular_drag);
	rigid_body->setLinearDamping(parameters.linear_drag);
	return rigid_body;
}

auto PhysicsEngine::add_box_collider(void* rigid_body, const ColliderParameters& parameters) -> void
{
	rp3d::BoxShape* shape = engine->common.createBoxShape(to_react_from_glm(parameters.half_size));
	auto* rp3d_body = static_cast<rp3d::RigidBody*>(rigid_body);
	rp3d::Transform transform = rp3d::Transform::identity();

	transform.setPosition(transform.getPosition() + to_react_from_glm(parameters.offset));

	rp3d_body->addCollider(shape, transform);
}

auto PhysicsEngine::add_sphere_collider(void* rigid_body, const ColliderParameters& parameters) -> void
{
	auto* rp3d_body = static_cast<rp3d::RigidBody*>(rigid_body);

	rp3d::SphereShape* shape = engine->common.createSphereShape(parameters.radius);
	rp3d::Transform transform = rp3d::Transform::identity();

	transform.setPosition(transform.getPosition() + to_react_from_glm(parameters.offset));

	rp3d_body->addCollider(shape, transform);
}

auto PhysicsEngine::add_capsule_collider(void* rigid_body, const ColliderParameters& parameters) -> void
{
	auto* rp3d_body = static_cast<rp3d::RigidBody*>(rigid_body);

	rp3d::CapsuleShape* shape = engine->common.createCapsuleShape(parameters.radius, parameters.height);
	rp3d::Transform transform = rp3d::Transform::identity();

	transform.setPosition(transform.getPosition() + to_react_from_glm(parameters.offset));

	rp3d_body->addCollider(shape, transform);
}

} // namespace Disarray
