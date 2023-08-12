#pragma once

#include "PropertySupplier.hpp"
#include "graphics/Shader.hpp"

namespace Disarray::Vulkan {

	class Shader : public Disarray::Shader, public PropertySupplier<VkPipelineShaderStageCreateInfo> {
	public:
		Shader(const Disarray::Device& device, const ShaderProperties&);
		~Shader() override;

		VkPipelineShaderStageCreateInfo supply() const override { return stage; }

		void destroy_module() override;

		const ShaderProperties& get_properties() const override { return props; }
		ShaderProperties& get_properties() override { return props; }

	private:
		std::string read_file(const std::filesystem::path&);

		bool was_destroyed_explicitly { false };

		const Disarray::Device& device;
		ShaderProperties props;

		VkPipelineShaderStageCreateInfo stage;
		VkShaderModule shader_module;
	};
} // namespace Disarray::Vulkan
