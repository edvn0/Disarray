#include "PC.glsl"
#include "UBO.glsl"
#include "CameraUBO.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex_coords;

layout(location = 0) out vec2 out_tex_coords;

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 1) uniform CameraUniformBlock { CameraUBO cbo; }
CBO;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

const vec2 fragment_offsets[6] = vec2[](vec2(-1, -1), vec2(-1, 1), vec2(1, -1), vec2(1, -1), vec2(-1, 1), vec2(1, 1));
const float radius = 1.0F;

void main()
{
	Uniform ubo = UBO.ubo;
	CameraUBO cbo = CBO.cbo;
	vec4 light_camera_space = cbo.view * vec4(in_pos, 1.0F);
	vec4 position_camera_space = light_camera_space + radius * vec4(in_tex_coords, 0, 0);
	gl_Position = ubo.proj * position_camera_space;

	out_tex_coords = in_tex_coords;
}
