#include "PC.glsl"
#include "ShadowPassUBO.glsl"
#include "UBO.glsl"
#include "MathHelpers.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;
layout(set = 0, binding = 3) uniform ShadowPassUniformBlock { ShadowPassUBO spu; }
SPU;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 colour;
layout(location = 3) in vec3 normals;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 output_normals;
layout(location = 3) out vec3 fragment_position;
layout(location = 4) out vec4 light_space_fragment_position;

void main()
{
    Uniform ubo = UBO.ubo;
    PushConstant pc = PC.pc;

    vec4 frag_pos = pc.object_transform * vec4(pos, 1.0);
    gl_Position = ubo.view_projection * frag_pos;

    light_space_fragment_position = bias_matrix() * SPU.spu.view_projection * frag_pos;
    fragment_position = vec3(frag_pos);

    fragColor = pc.colour;
    uvs = uv;
    output_normals = correct_normals(pc.object_transform, normals);
}
