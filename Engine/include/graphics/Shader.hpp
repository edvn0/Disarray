#pragma once

#include <filesystem>
#include <string_view>

#include "core/DisarrayObject.hpp"
#include "core/ReferenceCounted.hpp"
#include "core/Types.hpp"
#include "graphics/Device.hpp"

namespace Disarray {

enum class ShaderType { Vertex, Fragment, Compute };

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
	std::filesystem::path path;
	ShaderType type { ShaderType::Vertex };
	std::string entry_point = "main";
};

class Shader : public ReferenceCountable {
	DISARRAY_OBJECT(Shader)
public:
	virtual void destroy_module() = 0;
	virtual const ShaderProperties& get_properties() const = 0;
	virtual ShaderProperties& get_properties() = 0;

	static Ref<Disarray::Shader> construct(const Disarray::Device& device, const ShaderProperties&);
};

} // namespace Disarray
