#include "MathHelpers.glsl"
#include "PC.glsl"
#include "SSBODefinitions.glsl"
#include "ShadowPassUBO.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(std430, set = 3, binding = 2) readonly buffer Identifiers { IdentifierObject ssbo_objects[]; }
IdentifierSSBO;

layout(std140, set = 3, binding = 3) readonly buffer Transforms { TransformObject ssbo_objects[]; }
TransformSSBO;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

#include "DefaultInput.glsl"

layout(location = 0) out flat uint identifier;

void main()
{
	Uniform ubo = UBO.ubo;
	mat4 object_matrix = TransformSSBO.ssbo_objects[gl_InstanceIndex].transform;
	uint object_identifier = IdentifierSSBO.ssbo_objects[gl_InstanceIndex].identifier;

	vec4 frag_pos = object_matrix * vec4(pos, 1.0);
	gl_Position = ubo.view_projection * frag_pos;
	identifier = object_identifier;
}
