#include "core/Formatters.hpp"

#include <fmt/format.h>

auto fmt::formatter<glm::vec2>::format(glm::vec2 vec, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[{}, {}]", vec.x, vec.y), ctx);
}

auto fmt::formatter<glm::vec3>::format(glm::vec3 vec, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[{}, {}, {}]", vec.x, vec.y, vec.z), ctx);
}

auto fmt::formatter<glm::vec4>::format(glm::vec4 vec, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[{}, {}, {}, {}]", vec.x, vec.y, vec.z, vec.w), ctx);
}

auto fmt::formatter<glm::quat>::format(glm::quat vec, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[w={}, x={}, y={}, z={}]", vec.w, vec.x, vec.y, vec.z), ctx);
}

auto fmt::formatter<std::filesystem::path>::format(std::filesystem::path path, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[Path={}]", path.string()), ctx);
}
