#pragma once

#include <graphics/RendererProperties.hpp>

#include <utility>

#include "core/Collections.hpp"
#include "core/Ensure.hpp"
#include "graphics/Shader.hpp"
#include "graphics/UnifiedShader.hpp"
#include "vulkan/PropertySupplier.hpp"
#include "vulkan/Shader.hpp"

namespace Disarray::Vulkan {

class UnifiedShader final : public Disarray::UnifiedShader {
	DISARRAY_MAKE_NONCOPYABLE(UnifiedShader)

public:
	UnifiedShader(const Disarray::Device& device, UnifiedShaderProperties);
	~UnifiedShader() override;

	auto recreate(bool, const Extent&) -> void override;

	auto get_shader_buffers() const -> const auto& { return reflection_data.constant_buffers; }
	auto get_resources() const -> const auto& { return reflection_data.resources; }

private:
	auto clean_shader() -> void;
	auto create_vulkan_objects() -> void;
	auto create_descriptor_set_layouts() -> void;
	const Disarray::Device& device;

	std::vector<VkDescriptorSetLayout> descriptor_set_layouts {};
	VkDescriptorSet descriptor_set { nullptr };
	ReflectionData reflection_data;

	std::unordered_map<ShaderType, std::string> sources;
	std::unordered_map<ShaderType, std::vector<std::uint32_t>> spirv_sources;
	std::unordered_map<ShaderType, VkShaderModule> shader_modules;
	std::unordered_map<ShaderType, VkPipelineShaderStageCreateInfo> stage_infos;
};
} // namespace Disarray::Vulkan
