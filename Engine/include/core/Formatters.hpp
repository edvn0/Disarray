#pragma once

#include <glm/glm.hpp>

#include <fmt/core.h>

#include <filesystem>

#include "graphics/ImageProperties.hpp"

template <> struct fmt::formatter<Disarray::ImageFormat> : fmt::formatter<std::string_view> {
	auto format(const Disarray::ImageFormat& image_format, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<Disarray::Extent> : fmt::formatter<std::string_view> {
	auto format(const Disarray::Extent& extent, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<Disarray::FloatExtent> : fmt::formatter<std::string_view> {
	auto format(const Disarray::FloatExtent& extent, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<glm::vec2> : fmt::formatter<std::string_view> {
	auto format(const glm::vec2& vec, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<glm::vec3> : fmt::formatter<std::string_view> {
	auto format(const glm::vec3& vec, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<glm::vec4> : fmt::formatter<std::string_view> {
	auto format(const glm::vec4& vec, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<glm::quat> : fmt::formatter<std::string_view> {
	auto format(const glm::quat& vec, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<std::filesystem::path> : fmt::formatter<std::string_view> {
	auto format(const std::filesystem::path& path, format_context& ctx) -> decltype(ctx.out());
};

template <> struct fmt::formatter<std::exception> : fmt::formatter<std::string_view> {
	auto format(const std::exception& exc, format_context& ctx) -> decltype(ctx.out());
};
