#include "graphics/SingleShader.hpp"

#include "graphics/StaticMesh.hpp"
#include "vulkan/SingleShader.hpp"

auto std::hash<Disarray::SingleShader>::operator()(const Disarray::SingleShader& shader) const noexcept -> std::size_t
{
	return hash_value(shader.get_properties().path);
}

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
