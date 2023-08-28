#pragma once

#include <glm/glm.hpp>

#include <fmt/core.h>
#include <fmt/format.h>

#include <filesystem>

#include "graphics/ImageProperties.hpp"

template <> struct fmt::formatter<Disarray::Extent> : fmt::formatter<std::string_view> {
	auto format(const Disarray::Extent& ext, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<Disarray::FloatExtent> : fmt::formatter<std::string_view> {
	auto format(const Disarray::FloatExtent& ext, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<glm::vec2> : fmt::formatter<std::string_view> {
	auto format(const glm::vec2& format, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<glm::vec3> : fmt::formatter<std::string_view> {
	auto format(const glm::vec3& format, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<glm::vec4> : fmt::formatter<std::string_view> {
	auto format(const glm::vec4& format, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<glm::quat> : fmt::formatter<std::string_view> {
	auto format(const glm::quat& format, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<std::filesystem::path> : fmt::formatter<std::string_view> {
	auto format(const std::filesystem::path& format, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<std::exception> : fmt::formatter<std::string_view> {
	auto format(const std::exception& format, format_context& ctx) -> decltype(ctx.out());
};
