#include "PC.glsl"
#include "UBO.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex_coords;

layout(location = 0) out vec2 out_tex_coords;

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

const vec2 fragment_offsets[6] = vec2[](vec2(-1, -1), vec2(-1, 1), vec2(1, -1), vec2(1, -1), vec2(-1, 1), vec2(1, 1));

void main()
{
	Uniform ubo = UBO.ubo;
	vec3 cam_right_world = { ubo.view[0][0], ubo.view[1][0], ubo.view[2][0] };
	vec3 cam_up_world = { ubo.view[0][1], ubo.view[1][1], ubo.view[2][1] };

	vec3 y_offset = cam_up_world * fragment_offsets[gl_VertexIndex].y;
	vec3 x_offset = cam_right_world * fragment_offsets[gl_VertexIndex].x;
	vec3 vp_worldspace = in_pos + y_offset + x_offset;
	gl_Position = ubo.view_projection * vec4(vp_worldspace, 1.0);

	out_tex_coords = in_tex_coords;
}
