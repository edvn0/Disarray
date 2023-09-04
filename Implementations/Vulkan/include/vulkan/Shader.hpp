#pragma once

#include "PropertySupplier.hpp"
#include "graphics/Shader.hpp"

namespace Disarray::Vulkan {

class Shader : public Disarray::Shader, public PropertySupplier<VkPipelineShaderStageCreateInfo> {
public:
	Shader(const Disarray::Device& device, ShaderProperties);
	~Shader() override;

	auto supply() const -> VkPipelineShaderStageCreateInfo override { return stage; }
	void destroy_module() override;

private:
	static auto read_file(const std::filesystem::path&) -> std::string;

	bool was_destroyed_explicitly { false };

	const Disarray::Device& device;
	VkPipelineShaderStageCreateInfo stage {};
	VkShaderModule shader_module {};
};
} // namespace Disarray::Vulkan
