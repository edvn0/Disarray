#include "MathHelpers.glsl"
#include "PC.glsl"
#include "SSBODefinitions.glsl"
#include "ShadowPassUBO.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 3) uniform ShadowPassBlock { ShadowPassUBO spu; }
SPU;

// all object matrices
layout(std140, set = 3, binding = 4) readonly buffer Transforms { TransformObject ssbo_objects[]; }
TransformSSBO;

layout(std140, set = 3, binding = 5) readonly buffer Colours { ColourObject ssbo_objects[]; }
ColourSSBO;

#include "DefaultInput.glsl"

layout(location = 0) out vec4 fragment_colour;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 output_normals;
layout(location = 3) out vec3 fragment_position;
layout(location = 4) out vec4 light_space_fragment_position;

void main()
{
    Uniform ubo = UBO.ubo;
    mat4 object_matrix = TransformSSBO.ssbo_objects[gl_InstanceIndex].transform;
    vec4 object_colour = ColourSSBO.ssbo_objects[gl_InstanceIndex].colour;

    vec4 frag_pos = object_matrix * vec4(pos, 1.0);
    gl_Position = ubo.view_projection * frag_pos;

    light_space_fragment_position = bias_matrix() * SPU.spu.view_projection * object_matrix * vec4(pos, 1.0);
    fragment_position = vec3(frag_pos);

    fragment_colour = object_colour;
    uvs = uv;
    output_normals = correct_normals(object_matrix, normals);
}
