#pragma once

#include "graphics/Device.hpp"
#include "graphics/SingleShader.hpp"
namespace Disarray::Vulkan {

class SingleShader : public Disarray::SingleShader {
public:
	SingleShader(const Disarray::Device&, SingleShaderProperties);

	auto get_name() const -> std::string_view override;

private:
	const Disarray::Device& device;
	std::string name;
};

} // namespace Disarray::Vulkan
