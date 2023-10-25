#include "DisarrayPCH.hpp"

#include "graphics/Shader.hpp"

#include <spirv_reflect.hpp>

#include <fstream>

#include "core/Ensure.hpp"
#include "core/Formatters.hpp"
#include "graphics/ShaderCompiler.hpp"
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
		case ShaderType::Include:
			return VK_SHADER_STAGE_ALL;
		default:
			unreachable("Could not map to Vulkan stage");
		}
	}

	void create_module(const Vulkan::Device& device, const std::string& spirv_code, VkShaderModule& shader)
	{
		auto create_info = vk_structures<VkShaderModuleCreateInfo> {}();
		create_info.codeSize = spirv_code.size() * sizeof(std::uint32_t);
		create_info.pCode = Disarray::bit_cast<const uint32_t*>(spirv_code.data());

		verify(vkCreateShaderModule(*device, &create_info, nullptr, &shader));
	}

	void create_module(const Vulkan::Device& device, const std::vector<std::uint32_t>& spirv_code, VkShaderModule& shader)
	{
		auto create_info = vk_structures<VkShaderModuleCreateInfo> {}();
		create_info.codeSize = spirv_code.size() * sizeof(std::uint32_t);
		create_info.pCode = spirv_code.data();

		verify(vkCreateShaderModule(*device, &create_info, nullptr, &shader));
	}

	auto reflect_code(const std::vector<std::uint32_t>& spirv, ReflectionData& output)
	{
		spirv_cross::CompilerReflection reflection_compiler { spirv };
		spirv_cross::ShaderResources shader_resources = reflection_compiler.get_shader_resources();
		// Count the number of output attachments

		const auto& model = reflection_compiler.get_execution_model();

		for (const auto& resource : shader_resources.stage_outputs) {
			// Check if the resource is an output attachment
			if (reflection_compiler.get_decoration(resource.id, spv::Decoration::DecorationLocation) >= 0) {
				output.attachment_count++;
			}
		}

		// Technically, we only care about how many fragment outputs there are.
		// FIXME: Will need to generalise if / when we care about vertex outputs.
		if (model != spv::ExecutionModelFragment) {
			output.attachment_count = 0;
		}
		Log::info("SPIRV Reflection", "Attachment count: {}", output.attachment_count);
	}

} // namespace

Shader::Shader(const Disarray::Device& dev, ShaderProperties properties)
	: Disarray::Shader(std::move(properties))
	, device(dev)
{
	auto type = to_stage(props.type);

	if (props.code) {
		ensure(!props.identifier.empty(), "Must supply an identifier");
		reflect_code(*props.code, reflection_data);
		create_module(cast_to<Vulkan::Device>(device), *props.code, shader_module);
	} else {
		ensure(props.path.has_value(), "No code, but no path provided.");
		props.identifier = props.path.value();
		auto read = Shader::read_file(props.path.value());
		create_module(cast_to<Vulkan::Device>(device), read, shader_module);
	}

	stage = {};
	stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage.stage = type;
	stage.module = shader_module;
	stage.pName = props.entry_point.data();
}

Shader::Shader(const Disarray::Device& dev, const std::filesystem::path& path)
	: device(dev)
{
	static Runtime::ShaderCompiler compiler {};
	props.type = to_shader_type(path);
	props.path = path;
	auto type = to_stage(props.type);
	props.identifier = path;
	props.code = compiler.compile(path, props.type);
	if (props.code.has_value() && props.code->empty()) {
		Log::info("Shader", "Could not compile {}", path);
		throw CouldNotCompileShaderException { fmt::format("Path: {}", path) };
	}

	reflect_code(*props.code, reflection_data);

	create_module(cast_to<Vulkan::Device>(device), *props.code, shader_module);

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

auto Shader::attachment_count() const -> std::uint32_t
{
	if (props.type == ShaderType::Fragment) {
		return reflection_data.attachment_count;
	}
	return 0;
}

} // namespace Disarray::Vulkan
