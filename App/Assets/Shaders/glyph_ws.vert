#include "UBO.glsl"
#include "CameraUBO.glsl"
#include "SSBODefinitions.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex_coords;

layout(location = 0) out vec2 out_tex_coords;
layout(location = 1) out vec4 colour;
layout(location = 2) out flat uint font_image_identifier;

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 1) uniform CameraUniformBlock { CameraUBO cbo; }
CBO;

layout(std140, set = 3, binding = 6) readonly buffer Transforms { ColourImage ssbo_objects[]; }
ColourImageSSBO;

const float radius = 1.0F;

void main()
{
	Uniform ubo = UBO.ubo;
	CameraUBO cbo = CBO.cbo;
	vec4 light_camera_space = cbo.view * vec4(in_pos, 1.0F);
	vec4 position_camera_space = light_camera_space + radius * vec4(in_tex_coords, 0, 0);
	gl_Position = ubo.proj * position_camera_space;

	out_tex_coords = in_tex_coords;
	uint index = uint(gl_VertexIndex / 4);
	colour = ColourImageSSBO.ssbo_objects[index].colour;
	font_image_identifier = ColourImageSSBO.ssbo_objects[index].identifier;
}
