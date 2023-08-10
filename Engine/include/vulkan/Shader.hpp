#pragma once

#include "PropertySupplier.hpp"
#include "graphics/Shader.hpp"

namespace Disarray::Vulkan {

	class Shader : public Disarray::Shader, public PropertySupplier<VkPipelineShaderStageCreateInfo> {
	public:
		Shader(Disarray::Device& device, const ShaderProperties&);
		~Shader() override;

		VkPipelineShaderStageCreateInfo supply() const override { return stage; }

		void destroy_module() override;

		const std::string& path() const override { return shader_path; }

	private:
		std::string read_file(const std::filesystem::path&);

		bool was_destroyed_explicitly { false };

		Disarray::Device& device;
		ShaderProperties props;
		std::string shader_path;

		VkPipelineShaderStageCreateInfo stage;
		VkShaderModule shader_module;
	};
} // namespace Disarray::Vulkan
