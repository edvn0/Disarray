#include "DisarrayPCH.hpp"

#include "graphics/Shader.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Shader.hpp"
#include "vulkan/Structures.hpp"
#include "vulkan/Verify.hpp"

#include <bit>
#include <fstream>
#include <stdexcept>

namespace Disarray::Vulkan {

namespace {
	auto to_stage(ShaderType type)
	{
		switch (type) {
		case ShaderType::Vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderType::Fragment:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderType::Compute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		default:
			unreachable("Could not map to Vulkan stage");
		}
	}

	void create_module(const Vulkan::Device& device, const std::string& code, VkShaderModule& shader)
	{
		auto create_info = vk_structures<VkShaderModuleCreateInfo> {}();
		create_info.codeSize = code.size();
		create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

		verify(vkCreateShaderModule(*device, &create_info, nullptr, &shader));
	}
} // namespace

Shader::Shader(const Disarray::Device& dev, const ShaderProperties& properties)
	: device(dev)
	, props(properties)
{
	auto source = read_file(props.path);

	auto type = to_stage(props.type);

	create_module(cast_to<Vulkan::Device>(device), source, shader_module);

	stage = {};
	stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage.stage = type;
	stage.module = shader_module;
	stage.pName = props.entry_point.data();
}

Shader::~Shader()
{
	if (!was_destroyed_explicitly)
		vkDestroyShaderModule(supply_cast<Vulkan::Device>(device), shader_module, nullptr);
}

std::string Shader::read_file(const std::filesystem::path& path)
{
	std::ifstream stream { path.string().c_str(), std::ios::ate | std::ios::in | std::ios::binary };
	if (!stream) {
		throw CouldNotOpenStreamException("Could not open stream to file");
	}

	const std::size_t size = stream.tellg();
	std::vector<char> buffer {};
	buffer.resize(size);

	stream.seekg(0);
	stream.read(buffer.data(), size);

	return std::string { buffer.begin(), buffer.end() };
}

void Shader::destroy_module()
{
	vkDestroyShaderModule(supply_cast<Vulkan::Device>(device), shader_module, nullptr);
	was_destroyed_explicitly = true;
}

} // namespace Disarray::Vulkan
