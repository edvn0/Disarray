#include "graphics/SingleShader.hpp"

#include "graphics/StaticMesh.hpp"
#include "vulkan/SingleShader.hpp"

namespace Disarray {

auto SingleShader::construct(const Disarray::Device& device, SingleShaderProperties properties) -> Ref<Disarray::SingleShader>
{
	return make_ref<Vulkan::SingleShader>(device, properties);
}

auto SingleShader::construct_scoped(const Disarray::Device& device, SingleShaderProperties properties) -> Scope<Disarray::SingleShader>
{
	return make_scope<Vulkan::SingleShader>(device, properties);
}

} // namespace Disarray
