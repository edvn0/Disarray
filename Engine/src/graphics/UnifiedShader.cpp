#include "DisarrayPCH.hpp"

#include <fmt/format.h>
#include <magic_enum.hpp>

#include <streambuf>

#include "graphics/UnifiedShader.hpp"
#include "vulkan/UnifiedShader.hpp"

auto std::hash<Disarray::UnifiedShader>::operator()(const Disarray::UnifiedShader& shader) const noexcept -> std::size_t
{
	return hash_value(shader.get_properties().path);
}

namespace Disarray {

auto UnifiedShader::construct(const Disarray::Device& device, UnifiedShaderProperties properties) -> Ref<Disarray::UnifiedShader>
{
	return make_ref<Vulkan::UnifiedShader>(device, std::move(properties));
}

} // namespace Disarray
