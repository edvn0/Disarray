#include "PC.glsl"
#include "ShadowPassUBO.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 3) uniform ShadowPassUniformBlock { ShadowPassUBO spu; }
SPU;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normals;
layout(location = 3) in vec4 colour;

layout(location = 0) out vec4 output_colour;
layout(location = 1) out vec2 output_uvs;
layout(location = 2) out vec3 output_normals;
layout(location = 3) out vec3 output_fragment_pos;
layout(location = 4) out vec4 output_light_space_frag_pos;

const mat4 bias_matrix = mat4(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.5, 0.5, 0.0, 1.0);

void main()
{
	Uniform ubo = UBO.ubo;
	PushConstant pc = PC.pc;
	ShadowPassUBO spu = SPU.spu;

	vec4 object_position = pc.object_transform * vec4(pos, 1.0);

	gl_Position = ubo.view_projection * object_position;
	output_fragment_pos = vec3(object_position);
	output_light_space_frag_pos = bias_matrix * spu.view_projection * object_position;
	output_colour = colour;
	output_uvs = uv;
	output_normals = normalize(normals);
}
