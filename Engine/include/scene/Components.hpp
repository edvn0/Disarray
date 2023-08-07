#pragma once

#include "core/Concepts.hpp"
#include "core/UniquelyIdentifiable.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Disarray {

	namespace {
		static const auto default_rotation = glm::angleAxis(0.f, glm::vec3 { 0.f, 0.f, 1.0f });
		static constexpr auto identity = glm::identity<glm::mat4>();

		inline auto scale_matrix(const auto& vec) { return glm::scale(identity, vec); }

		inline auto translate_matrix(const auto& vec) { return glm::translate(identity, vec); }
	} // namespace

	struct Transform {
		glm::quat rotation { default_rotation };
		glm::vec3 position { 0.0f };
		glm::vec3 scale { 1.0f };

		Transform() = default;
		Transform(const glm::vec3& euler, const glm::vec3& pos, const glm::vec3& scl)
			: rotation(euler)
			, position(pos)
			, scale(scl)
		{
		}

		const auto compute() const { return translate_matrix(position) * glm::mat4_cast(rotation) * scale_matrix(scale); }
	};

	struct ID {
		Identifier identifier {};

		std::uint32_t get_identifier() const { return static_cast<std::uint32_t>(identifier); }
	};

} // namespace Disarray
