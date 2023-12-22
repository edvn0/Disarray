#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <magic_enum.hpp>
#include <nlohmann/json.hpp>

#include "core/Concepts.hpp"
#include "scene/ComponentSerialisers.hpp"
#include "scene/CppScript.hpp"

namespace Disarray {

template <class Enum> inline auto to_enum_value(const auto& object, std::string_view key) -> decltype(auto)
{
	std::string value;
	if (!object.contains(key)) {
		return std::optional<Enum>(std::nullopt);
	}
	object[key].get_to(value);
	return magic_enum::enum_cast<Enum>(value);
}
using json = nlohmann::json;

void to_json(json&, const Extent&);
void from_json(const json&, Extent&);

void to_json(json&, const FloatExtent&);
void from_json(const json&, FloatExtent&);

} // namespace Disarray

NLOHMANN_JSON_NAMESPACE_BEGIN
template <glm::length_t T> struct adl_serializer<glm::vec<T, float>> {
	static_assert(T > 0 && T <= 4);
	static void to_json(json& object, const glm::vec<T, float>& vec)
	{
		if constexpr (T == 1) {
			object = json::array({ vec[0] });
		}
		if constexpr (T == 2) {
			object = json::array({ vec[0], vec[1] });
		}
		if constexpr (T == 3) {
			object = json::array({ vec[0], vec[1], vec[2] });
		}
		if constexpr (T == 4) {
			object = json::array({ vec[0], vec[1], vec[2], vec[3] });
		}
	}

	static void from_json(const json& object, glm::vec<T, float>& opt)
	{
		if constexpr (T == 1) {
			opt = glm::vec1 { object[0] };
		}
		if constexpr (T == 2) {
			opt = glm::vec2 { object[0], object[1] };
		}
		if constexpr (T == 3) {
			opt = glm::vec3 { object[0], object[1], object[2] };
		}
		if constexpr (T == 4) {
			opt = glm::vec4 { object[0], object[1], object[2], object[3] };
		}
	}
};

template <> struct adl_serializer<glm::quat> {
	static void to_json(json& object, const glm::quat& p) { object = json::array({ "q", p.w, p.x, p.y, p.z }); }
	static void from_json(const json& object, glm::quat& opt) { opt = glm::quat(object[1], object[2], object[3], object[4]); }
};

template <glm::length_t N> struct adl_serializer<glm::mat<N, N, float>> {
	static void to_json(json& object, const glm::mat<N, N, float>& path)
	{
		if constexpr (N == 2) {
			object = json::array({ path[0], path[1] });
		}
		if constexpr (N == 3) {
			object = json::array({ path[0], path[1], path[2] });
		}
		if constexpr (N == 4) {
			object = json::array({ path[0], path[1], path[2], path[3] });
		}
	}
	static void from_json(const json& object, glm::mat<N, N, float>& opt)
	{
		if constexpr (N == 2) {
			opt = glm::mat2 { object[0], object[1] };
		}
		if constexpr (N == 3) {
			opt = glm::mat3 { object[0], object[1], object[2] };
		}
		if constexpr (N == 4) {
			opt = glm::mat4 { object[0], object[1], object[2], object[3] };
		}
	}
};

template <> struct adl_serializer<std::filesystem::path> {
	static void to_json(json& object, const std::filesystem::path& path) { object = std::filesystem::relative(path).string(); }
	static void from_json(const json& object, std::filesystem::path& opt)
	{
		std::string filename;
		object.get_to(filename);
		opt = std::filesystem::path(filename);
	}
};

template <> struct adl_serializer<Disarray::Parameter> {
	static void to_json(json& object, const Disarray::Parameter& param);
	static void from_json(const json& object, Disarray::Parameter& param);
};
NLOHMANN_JSON_NAMESPACE_END
