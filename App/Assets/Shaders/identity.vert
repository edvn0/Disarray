#include "MathHelpers.glsl"
#include "PC.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

#include "DefaultInput.glsl"

void main()
{
	Uniform ubo = UBO.ubo;
	PushConstant pc = PC.pc;

	vec4 model_position = pc.object_transform * vec4(pos, 1.0);
	gl_Position = ubo.view_projection * model_position;
}
