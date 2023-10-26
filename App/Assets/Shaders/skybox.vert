#include "UBO.glsl"
#include "PC.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout (location = 0) in vec3 pos;

layout(location = 0) out vec3 uvs;

void main()
{
	vec4 position = PC.pc.object_transform * vec4(pos, 1.0);
    gl_Position = UBO.ubo.view_projection * position;
    uvs = vec3(position);
    uvs.xy *= -1.0;
}
