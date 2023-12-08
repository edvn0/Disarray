#include "DisarrayPCH.hpp"

#include "graphics/Pipeline.hpp"
#include "vulkan/Pipeline.hpp"

namespace Disarray {

auto PipelineProperties::hash() const -> std::size_t
{
	std::size_t seed { 0 };
	hash_combine(seed, extent.width, extent.height, cull_mode, face_mode, depth_comparison_operator, samples, write_depth, test_depth, line_width);

	if (vertex_shader) {
		hash_combine(seed, *vertex_shader);
	}

	if (fragment_shader) {
		hash_combine(seed, *fragment_shader);
	}
	return seed;
}

void PipelineProperties::set_shader_with_type(ShaderType type, const Ref<Disarray::Shader>& shader)
{
	switch (type) {
	case ShaderType::Vertex: {
		vertex_shader = shader;
		break;
	}
	case ShaderType::Fragment: {
		fragment_shader = shader;
		break;
	}
	case ShaderType::Compute: {
		compute_shader = shader;
		break;
	}
	case ShaderType::Include:
		break;
	}
}

auto PipelineProperties::is_valid() const -> bool { return framebuffer != nullptr; }

auto Pipeline::construct(const Disarray::Device& device, Disarray::PipelineProperties properties) -> Ref<Disarray::Pipeline>
{
	return make_ref<Vulkan::Pipeline>(device, std::move(properties));
}

auto Pipeline::construct_scoped(const Disarray::Device& device, Disarray::PipelineProperties properties) -> Scope<Disarray::Pipeline>
{
	return make_scope<Vulkan::Pipeline>(device, std::move(properties));
}

} // namespace Disarray
