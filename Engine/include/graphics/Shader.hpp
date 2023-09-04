#pragma once

#include <fmt/core.h>

#include <filesystem>
#include <optional>
#include <string_view>
#include <vector>

#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/Device.hpp"

namespace Disarray {

enum class ShaderType : std::uint8_t { Vertex, Fragment, Compute };

static constexpr auto shader_type_extension(ShaderType shader_type)
{
	switch (shader_type) {
	case ShaderType::Vertex:
		return ".vert";
	case ShaderType::Fragment:
		return ".frag";
	case ShaderType::Compute:
		return ".comp";
	default:
		unreachable();
	}
}

inline auto to_shader_type(const std::filesystem::path& path_like)
{
	auto copy = path_like;
	if (path_like.extension() == ".spv") {
		copy.replace_extension();
	}

	if (copy.extension() == ".vert") {
		return ShaderType::Vertex;
	}
	if (copy.extension() == ".frag") {
		return ShaderType::Fragment;
	}
	if (copy.extension() == ".comp") {
		return ShaderType::Compute;
	}

	unreachable();
}

struct ShaderProperties {
	std::optional<std::vector<std::uint32_t>> code;
	std::filesystem::path path;
	ShaderType type { ShaderType::Vertex };
	std::string entry_point = "main";
};

class Shader : public ReferenceCountable {
	DISARRAY_OBJECT_PROPS(Shader, ShaderProperties)
public:
	virtual void destroy_module() = 0;

	static auto construct(const Disarray::Device& device, const ShaderProperties&) -> Ref<Disarray::Shader>;
};

} // namespace Disarray

namespace fmt {
template <> struct fmt::formatter<Disarray::ShaderType> : fmt::formatter<std::string_view> {
	auto format(const Disarray::ShaderType& format, format_context& ctx) -> decltype(ctx.out());
};

} // namespace fmt
