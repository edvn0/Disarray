#include "PC.glsl"
#include "UBO.glsl"
#include "ShadowPassUBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock {
	Uniform ubo;
} UBO;

layout(set = 0, binding = 3) uniform ShadowPassUniformBlock {
	ShadowPassUBO spu;
} SPU;

layout(push_constant) uniform PushConstantBlock
{
	PushConstant pc;
}
PC;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normals;
layout(location = 3) in vec4 colour;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 outNormals;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out vec4 lightSpaceFragPos;

void main() {
	Uniform ubo = UBO.ubo;
	PushConstant pc = PC.pc;
	ShadowPassUBO spu = SPU.spu;

	vec4 model_position = pc.object_transform * vec4(pos, 1.0);

	fragPos = vec3(model_position);
	gl_Position = ubo.view_projection * model_position;
	lightSpaceFragPos = spu.view_projection * model_position;
	fragColor = colour;
	uvs = uv;
	outNormals = normalize(normals);
}

