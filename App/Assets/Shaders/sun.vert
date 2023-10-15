#include "PC.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 colour;
layout(location = 3) in vec3 normals;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;

layout(location = 0) out vec4 fragment_colour;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 output_normals;

void main()
{
	Uniform ubo = UBO.ubo;
	PushConstant pc = PC.pc;

	gl_Position = ubo.view_projection * pc.object_transform * vec4(pos, 1.0);
	fragment_colour = pc.colour;
	uvs = uv;
	output_normals = correct_normals(pc.object_transform, normals);
}
