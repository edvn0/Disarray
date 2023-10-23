#include "MathHelpers.glsl"
#include "PC.glsl"
#include "SSBODefinitions.glsl"
#include "ShadowPassUBO.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 3) uniform ShadowPassBlock { ShadowPassUBO spu; }
SPU;

layout(std140, set = 3, binding = 0) readonly buffer Transforms { TransformObject ssbo_objects[]; }
TransformSSBO;

layout(std140, set = 3, binding = 1) readonly buffer Colours { ColourObject ssbo_objects[]; }
ColourSSBO;

#include "DefaultInput.glsl"

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 outNormals;

void main()
{
	Uniform ubo = UBO.ubo;
	ShadowPassUBO spu = SPU.spu;

	mat4 vp = spu.view_projection;

	mat4 object_matrix = TransformSSBO.ssbo_objects[gl_InstanceIndex].transform;
	vec4 object_colour = ColourSSBO.ssbo_objects[gl_InstanceIndex].colour;

	vec4 model_matrix = object_matrix * vec4(pos, 1.0);
	gl_Position = vp * model_matrix;
	fragColor = object_colour;
	uvs = uv;
	outNormals = correct_normals(object_matrix, normals);
}
