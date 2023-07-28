#include "graphics/Shader.hpp"

#include "vulkan/Shader.hpp"

namespace Disarray {

	Ref<Shader> Shader::construct(Ref<Device> device, const ShaderProperties& props)
	{
		return make_ref<Vulkan::Shader>(device, props);
	}

}