#include "vulkan/SingleShader.hpp"

#include "graphics/SingleShader.hpp"

namespace Disarray::Vulkan {

SingleShader::SingleShader(const Disarray::Device& dev, SingleShaderProperties properties)
	: Disarray::SingleShader(std::move(properties))
	, device(dev)
	, name(props.path.filename().string())
{
}

auto SingleShader::get_name() const -> std::string_view { return name; }

} // namespace Disarray::Vulkan
