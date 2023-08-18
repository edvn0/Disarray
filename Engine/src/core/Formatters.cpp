#include "core/Formatters.hpp"

#include <fmt/format.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

auto fmt::formatter<const glm::vec2>::format(const glm::vec2& vec, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[{}, {}]", vec.x, vec.y), ctx);
}

auto fmt::formatter<const glm::vec3>::format(const glm::vec3& vec, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[{}, {}, {}]", vec.x, vec.y, vec.z), ctx);
}

auto fmt::formatter<const glm::vec4>::format(const glm::vec4& vec, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[{}, {}, {}, {}]", vec.x, vec.y, vec.z, vec.w), ctx);
}

auto fmt::formatter<const glm::quat>::format(const glm::quat& vec, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[w={}, x={}, y={}, z={}]", vec.w, vec.x, vec.y, vec.z), ctx);
}

auto fmt::formatter<std::filesystem::path>::format(const std::filesystem::path& path, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("{}", path.string()), ctx);
}
