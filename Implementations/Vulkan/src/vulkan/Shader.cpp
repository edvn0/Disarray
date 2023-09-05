#include "DisarrayPCH.hpp"

#include "graphics/Shader.hpp"

#include <fstream>
#include <stdexcept>

#include "core/Ensure.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Shader.hpp"
#include "vulkan/Structures.hpp"
#include "vulkan/Verify.hpp"

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
		create_info.codeSize = code.size() * sizeof(std::uint32_t);
		create_info.pCode = Disarray::bit_cast<const uint32_t*>(code.data());

		verify(vkCreateShaderModule(*device, &create_info, nullptr, &shader));
	}

	void create_module(const Vulkan::Device& device, const std::vector<std::uint32_t>& code, VkShaderModule& shader)
	{
		auto create_info = vk_structures<VkShaderModuleCreateInfo> {}();
		create_info.codeSize = code.size() * sizeof(std::uint32_t);
		create_info.pCode = code.data();

		verify(vkCreateShaderModule(*device, &create_info, nullptr, &shader));
	}
} // namespace

Shader::Shader(const Disarray::Device& dev, ShaderProperties properties)
	: Disarray::Shader(std::move(properties))
	, device(dev)
{
	auto type = to_stage(props.type);

	if (props.code) {
		ensure(!props.identifier.empty(), "Must supply an identifier");
		create_module(cast_to<Vulkan::Device>(device), *props.code, shader_module);
	} else {
		props.identifier = props.path;
		auto read = Shader::read_file(props.path);
		create_module(cast_to<Vulkan::Device>(device), read, shader_module);
	}

	stage = {};
	stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage.stage = type;
	stage.module = shader_module;
	stage.pName = props.entry_point.data();
}

Shader::~Shader()
{
	if (!was_destroyed_explicitly) {
		vkDestroyShaderModule(supply_cast<Vulkan::Device>(device), shader_module, nullptr);
	}
}

auto Shader::read_file(const std::filesystem::path& path) -> std::string
{
	std::ifstream stream { path.string().c_str(), std::ios::ate | std::ios::in | std::ios::binary };
	if (!stream) {
		throw CouldNotOpenStreamException("Could not open stream to file");
	}

	const std::size_t size = stream.tellg();
	std::vector<char> buffer {};
	buffer.resize(size);

	stream.seekg(0);
	stream.read(buffer.data(), static_cast<long long>(size));

	return std::string { buffer.begin(), buffer.end() };
}

void Shader::destroy_module()
{
	vkDestroyShaderModule(supply_cast<Vulkan::Device>(device), shader_module, nullptr);
	was_destroyed_explicitly = true;
}

} // namespace Disarray::Vulkan
