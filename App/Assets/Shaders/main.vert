#include "MathHelpers.glsl"
#include "PC.glsl"
#include "UBO.glsl"

#define NAME MAIN

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

#include "DefaultInput.glsl"

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 output_normals;

void main()
{
	Uniform ubo = UBO.ubo;
	PushConstant pc = PC.pc;

	vec4 position = pc.object_transform * vec4(pos, 1.0);
	gl_Position = ubo.view_projection * position;
	fragColor = pc.colour;
	uvs = uv;
	output_normals = correct_normals(pc.object_transform, normals);
}
