#include "DisarrayPCH.hpp"

#include "vulkan/Shader.hpp"

#include "graphics/Shader.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Verify.hpp"
#include "vulkan/vulkan_core.h"

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
			default:
				throw std::runtime_error("Could not map to Vulkan stage");
			}
		}

		void create_module(Vulkan::Device& device, const std::string& code, VkShaderModule& shader)
		{
			VkShaderModuleCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			create_info.codeSize = code.size();
			create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

			verify(vkCreateShaderModule(*device, &create_info, nullptr, &shader));
		}
	} // namespace

	Shader::Shader(Disarray::Device& dev, const ShaderProperties& properties)
		: device(dev)
		, props(properties)
	{
		auto source = read_file(props.path);
		shader_path = props.path.string();

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
		std::ifstream stream { path, std::ios::ate | std::ios::in | std::ios::binary };
		if (!stream) {
			throw std::runtime_error("Could not open stream to file");
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
