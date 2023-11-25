#include "DisarrayPCH.hpp"

#include <fmt/format.h>
#include <magic_enum.hpp>

#include <streambuf>

#include "graphics/UnifiedShader.hpp"
#include "vulkan/UnifiedShader.hpp"

namespace Disarray {

auto UnifiedShader::construct(const Disarray::Device& device, UnifiedShaderProperties properties) -> Ref<Disarray::UnifiedShader>
{
	return make_ref<Vulkan::UnifiedShader>(device, std::move(properties));
}

} // namespace Disarray
