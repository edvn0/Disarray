#include "MathHelpers.glsl"
#include "PC.glsl"
#include "ShadowPassUBO.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 3) uniform ShadowPassUniformBlock { ShadowPassUBO spu; }
SPU;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

#include "DefaultInput.glsl"

layout(location = 0) flat out vec4 fragment_colour;
layout(location = 1) out vec2 uvs;
layout(location = 2) flat out vec3 out_normals;
layout(location = 3) out vec3 frag_pos;
layout(location = 4) out vec4 light_space_frag_pos;

void main()
{
	Uniform ubo = UBO.ubo;
	PushConstant pc = PC.pc;
	ShadowPassUBO spu = SPU.spu;

	vec4 model_position = pc.object_transform * vec4(pos, 1.0);

	frag_pos = vec3(model_position);
	gl_Position = ubo.view_projection * model_position;
	light_space_frag_pos = bias_matrix() * spu.view_projection * model_position;
	fragment_colour = colour;
	uvs = uv;
	out_normals = correct_normals(pc.object_transform, normals);
}
