#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace Disarray {

	using json = nlohmann::json;

	void to_json(json& j, const Extent& p)
	{
		j = json {
			{ "width", p.width },
			{ "height", p.height },
		};
	}
	void from_json(const json& j, Extent& p)
	{
		j.at("width").get_to(p.width);
		j.at("height").get_to(p.height);
	}

	void to_json(json& j, const FloatExtent& p)
	{
		j = json {
			{ "width", p.width },
			{ "height", p.height },
		};
	}
	void from_json(const json& j, FloatExtent& p)
	{
		j.at("width").get_to(p.width);
		j.at("height").get_to(p.height);
	}

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

template <> struct adl_serializer<std::filesystem::path> {
	static void to_json(json& j, const std::filesystem::path& p) { j = p.string(); }
	static void from_json(const json& j, std::filesystem::path& opt) { opt = to_string(j); }
};
NLOHMANN_JSON_NAMESPACE_END
