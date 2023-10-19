#include "PC.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(location = 0) in vec4 fragment_colour;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;

layout(location = 0) out vec4 colour;

void main()
{
	PushConstant pc = PC.pc;
	colour = fragment_colour;
}
