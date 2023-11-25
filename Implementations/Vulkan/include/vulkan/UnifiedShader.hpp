#pragma once

#include <glslang/Public/ShaderLang.h>

#include <utility>

#include "core/Collections.hpp"
#include "graphics/Shader.hpp"
#include "graphics/UnifiedShader.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

class UnifiedIncluder final : public glslang::TShader::Includer {
public:
	UnifiedIncluder(const std::filesystem::path& dir);
	~UnifiedIncluder() override;

	IncludeResult* includeSystem(const char*, const char*, std::size_t) override;
	IncludeResult* includeLocal(const char*, const char*, std::size_t) override;
	void releaseInclude(IncludeResult*) override;

private:
	auto get_or_emplace_include(const std::string& header_name) -> const std::string&;

	Collections::StringMap<std::string> sources {};
	std::filesystem::path directory;
};

class UnifiedShader final : public Disarray::UnifiedShader {
	DISARRAY_MAKE_NONCOPYABLE(UnifiedShader)
public:
	UnifiedShader(const Disarray::Device& device, UnifiedShaderProperties);
	~UnifiedShader() override;

private:
	std::vector<VkDescriptorSetLayout> descriptor_set_layouts {};
	VkDescriptorSet descriptor_set { nullptr };

	std::unordered_map<uint32_t, std::vector<VkDescriptorPoolSize>> type_counts {};

	const Disarray::Device& device;
	VkPipelineShaderStageCreateInfo stage {};
	VkShaderModule shader_module {};

	std::unordered_map<ShaderType, std::string> sources;
	std::unordered_map<ShaderType, std::vector<std::uint32_t>> spirv_sources;
};
} // namespace Disarray::Vulkan
