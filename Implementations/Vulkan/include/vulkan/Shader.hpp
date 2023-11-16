#pragma once

#include "graphics/Shader.hpp"
#include "vulkan/PropertySupplier.hpp"

namespace Disarray::Vulkan {

namespace Reflection {
	struct Uniform {
		std::uint32_t binding { 0 };
		std::uint32_t set { 0 };
	};

} // namespace Reflection

struct ReflectionData {
	std::uint32_t attachment_count { 0 };
};

class Shader : public Disarray::Shader, public PropertySupplier<VkPipelineShaderStageCreateInfo> {
	DISARRAY_MAKE_NONCOPYABLE(Shader)
public:
	Shader(const Disarray::Device& device, ShaderProperties);
	Shader(const Disarray::Device& device, const std::filesystem::path&);
	~Shader() override;

	auto attachment_count() const -> std::uint32_t override;

	[[nodiscard]] auto supply() const -> VkPipelineShaderStageCreateInfo override { return stage; }
	[[nodiscard]] auto supply() -> VkPipelineShaderStageCreateInfo override { return stage; }

	void destroy_module() override;

private:
	static auto read_file(const std::filesystem::path&) -> std::string;

	bool was_destroyed_explicitly { false };

	ReflectionData reflection_data {};

	const Disarray::Device& device;
	VkPipelineShaderStageCreateInfo stage {};
	VkShaderModule shader_module {};
};
} // namespace Disarray::Vulkan
