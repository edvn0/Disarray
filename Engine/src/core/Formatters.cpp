#include "core/Formatters.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <fmt/format.h>
#include <magic_enum.hpp>

auto fmt::formatter<Disarray::ImageFormat>::format(const Disarray::ImageFormat& image_format, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[{}]", magic_enum::enum_name(image_format)), ctx);
}

auto fmt::formatter<Disarray::Extent>::format(const Disarray::Extent& extent, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[{}, {}]", extent.width, extent.height), ctx);
}

auto fmt::formatter<Disarray::FloatExtent>::format(const Disarray::FloatExtent& extent, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[{}, {}]", extent.width, extent.height), ctx);
}

auto fmt::formatter<glm::vec2>::format(const glm::vec2& vec, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[{}, {}]", vec.x, vec.y), ctx);
}

auto fmt::formatter<glm::vec3>::format(const glm::vec3& vec, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[{}, {}, {}]", vec.x, vec.y, vec.z), ctx);
}

auto fmt::formatter<glm::vec4>::format(const glm::vec4& vec, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[{}, {}, {}, {}]", vec.x, vec.y, vec.z, vec.w), ctx);
}

auto fmt::formatter<glm::quat>::format(const glm::quat& vec, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("[w={}, x={}, y={}, z={}]", vec.w, vec.x, vec.y, vec.z), ctx);
}

auto fmt::formatter<std::filesystem::path>::format(const std::filesystem::path& path, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("{}", path.string()), ctx);
}

auto fmt::formatter<std::exception>::format(const std::exception& exc, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("{}", exc.what()), ctx);
}
