#include "graphics/RendererProperties.hpp"

namespace Disarray {

void UBO::reset_impl()
{
	static constexpr auto identity_matrix = glm::identity<glm::mat4>();
	static constexpr auto one_vector = glm::vec4(1, 1, 1, 1);
	view = identity_matrix;
	proj = identity_matrix;
	view_projection = identity_matrix;
	sun_direction_and_intensity = one_vector;
	sun_colour = one_vector;
}

void CameraUBO::reset_impl() { }

void ImageIndicesUBO::reset_impl()
{
	image_indices.fill({});
	bound_textures = 0;
}

} // namespace Disarray