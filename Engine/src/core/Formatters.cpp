#include "core/Formatters.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include <fmt/format.h>
#include <magic_enum.hpp>

#define GLM_SERIALISATION(GLMType)                                                                                                                   \
	auto fmt::formatter<GLMType>::format(const GLMType& param, fmt::format_context& ctx)->decltype(ctx.out())                                        \
	{                                                                                                                                                \
		return formatter<std::string_view>::format(fmt::format("{}", glm::to_string(param)), ctx);                                                   \
	}

GLM_SERIALISATION(glm::quat);
GLM_SERIALISATION(glm::vec2);
GLM_SERIALISATION(glm::vec3);
GLM_SERIALISATION(glm::vec4);
GLM_SERIALISATION(glm::mat2);
GLM_SERIALISATION(glm::mat3);
GLM_SERIALISATION(glm::mat4);

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

auto fmt::formatter<std::filesystem::path>::format(const std::filesystem::path& path, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("{}", path.string()), ctx);
}

auto fmt::formatter<std::exception>::format(const std::exception& exc, format_context& ctx) -> decltype(ctx.out())
{
	return formatter<std::string_view>::format(fmt::format("{}", exc.what()), ctx);
}
