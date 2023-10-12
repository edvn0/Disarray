#include "PC.glsl"
#include "ShadowPassUBO.glsl"
#include "UBO.glsl"
#include "MathHelpers.glsl"

#define NAME SHADOW

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 3) uniform ShadowPassBlock { ShadowPassUBO spu; }
SPU;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 colour;
layout(location = 3) in vec3 normals;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 outNormals;

void main()
{
    Uniform ubo = UBO.ubo;
    PushConstant pc = PC.pc;
    ShadowPassUBO spu = SPU.spu;

    mat4 vp = spu.view_projection;

    vec4 model_matrix = pc.object_transform * vec4(pos, 1.0);
    gl_Position = vp * model_matrix;
    fragColor = pc.colour;
    uvs = uv;
    outNormals = correct_normals(pc.object_transform, normals);
}
