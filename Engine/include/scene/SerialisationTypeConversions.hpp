#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <magic_enum.hpp>
#include <nlohmann/json.hpp>

#include "core/Concepts.hpp"
#include "scene/ComponentSerialisers.hpp"

namespace Disarray {

struct FloatExtent;
struct Extent;

template <class Enum> inline auto to_enum_value(const auto& object, std::string_view key) -> decltype(auto)
{
	std::string value;
	if (!object.contains(key)) {
		throw ComponentDeserialiseException { "ToEnumValue", fmt::format("Key {} was missing from object", key) };
	}
	object[key].get_to(value);
	return magic_enum::enum_cast<Enum>(value);
}
using json = nlohmann::json;

void to_json(json& j, const Extent& p);
void from_json(const json& j, Extent& p);

void to_json(json& j, const FloatExtent& p);
void from_json(const json& j, FloatExtent& p);

} // namespace Disarray

NLOHMANN_JSON_NAMESPACE_BEGIN
template <std::size_t T> struct adl_serializer<glm::vec<T, float>> {
	static_assert(T > 0 && T <= 4);
	static void to_json(json& j, const glm::vec<T, float>& vec)
	{
		if constexpr (T == 1) {
			j = json::array({ vec[0] });
		}
		if constexpr (T == 2) {
			j = json::array({ vec[0], vec[1] });
		}
		if constexpr (T == 3) {
			j = json::array({ vec[0], vec[1], vec[2] });
		}
		if constexpr (T == 4) {
			j = json::array({ vec[0], vec[1], vec[2], vec[3] });
		}
	}

	static void from_json(const json& j, glm::vec<T, float>& opt)
	{
		if constexpr (T == 1) {
			opt = glm::vec1 { j[0] };
		}
		if constexpr (T == 2) {
			opt = glm::vec2 { j[0], j[1] };
		}
		if constexpr (T == 3) {
			opt = glm::vec3 { j[0], j[1], j[2] };
		}
		if constexpr (T == 4) {
			opt = glm::vec4 { j[0], j[1], j[2], j[3] };
		}
	}
};

template <> struct adl_serializer<glm::quat> {
	static void to_json(json& j, const glm::quat& p) { j = json::array({ "q", p.w, p.x, p.y, p.z }); }
	static void from_json(const json& j, glm::quat& opt) { opt = glm::quat(j[1], j[2], j[3], j[4]); }
};

template <std::size_t N> struct adl_serializer<glm::mat<N, N, float>> {
	static void to_json(json& j, const glm::mat<N, N, float>& p)
	{
		if constexpr (N == 2) {
			j = json::array({ p[0], p[1] });
		}
		if constexpr (N == 3) {
			j = json::array({ p[0], p[1], p[2] });
		}
		if constexpr (N == 4) {
			j = json::array({ p[0], p[1], p[2], p[3] });
		}
	}
	static void from_json(const json& j, glm::mat<N, N, float>& opt)
	{
		if constexpr (N == 2) {
			opt = glm::mat2 { j[0], j[1] };
		}
		if constexpr (N == 3) {
			opt = glm::mat3 { j[0], j[1], j[2] };
		}
		if constexpr (N == 4) {
			opt = glm::mat4 { j[0], j[1], j[2], j[3] };
		}
	}
};

template <> struct adl_serializer<std::filesystem::path> {
	static void to_json(json& j, const std::filesystem::path& p) { j = std::filesystem::relative(p).string(); }
	static void from_json(const json& j, std::filesystem::path& opt)
	{
		std::string filename;
		j.get_to(filename);
		opt = std::filesystem::path(filename);
	}
};
NLOHMANN_JSON_NAMESPACE_END
