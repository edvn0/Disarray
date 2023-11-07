#include "CameraUBO.glsl"
#include "PC.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 1) uniform CameraBlock { CameraUBO cbo; }
CBO;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(location = 0) in vec3 pos;

layout(location = 0) out vec3 uvs;

void main()
{
	vec4 position = UBO.ubo.proj * CBO.cbo.view * vec4(pos, 1.0);
	gl_Position = position.xyww;
	uvs = pos;
	uvs.xy *= -1.0;
}
