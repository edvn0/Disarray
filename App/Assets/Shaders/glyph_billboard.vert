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
	mat3 view = mat3(cbo.view);
	vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 up = vec3(view[0][1], view[1][1], view[2][1]);

	vec3 calculated_position = in_pos - right * in_tex_coords.x - up * in_tex_coords.y;

	gl_Position = ubo.proj * cbo.view * vec4(calculated_position, 1.0F);

	out_tex_coords = in_tex_coords;
	uint index = uint(gl_VertexIndex / 4);
	colour = ColourImageSSBO.ssbo_objects[index].colour;
	font_image_identifier = ColourImageSSBO.ssbo_objects[index].identifier;
}
