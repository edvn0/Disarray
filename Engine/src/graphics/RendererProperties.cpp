#include "graphics/RendererProperties.hpp"

namespace Disarray {

void PushConstant::reset_impl()
{
	object_transform = glm::mat4 { 1.0F };
	colour = glm::vec4 { 1.0F };
	max_identifiers = 0;
	current_identifier = 0;
	max_point_lights = 0;
	bound_textures = 0;
	image_indices.fill(-1);
}

void UBO::reset_impl()
{
	static constexpr auto identity_matrix = glm::identity<glm::mat4>();
	static constexpr auto one_vector = glm::vec4(1, 1, 1, 1);
	view = identity_matrix;
	proj = identity_matrix;
	view_projection = identity_matrix;
}

void CameraUBO::reset_impl() { }

void ImageIndicesUBO::reset_impl()
{
	image_indices.fill({});
	bound_textures = 0;
}

} // namespace Disarray
