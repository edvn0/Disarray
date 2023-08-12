#include "DisarrayPCH.hpp"

#include "graphics/Shader.hpp"

#include "vulkan/Shader.hpp"

namespace Disarray {

	Ref<Shader> Shader::construct(const Disarray::Device& device, const ShaderProperties& props) { return make_ref<Vulkan::Shader>(device, props); }

} // namespace Disarray
