#include "MathHelpers.glsl"
#include "PC.glsl"
#include "ShadowPassUBO.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 3) uniform ShadowPassUniformBlock { ShadowPassUBO spu; }
SPU;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normals;
layout(location = 3) in vec4 colour;
layout(location = 4) in uint inIdentifier;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 outNormals;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out flat uint outIdentifier;
layout(location = 5) out vec4 light_space_fragment_position;

void main()
{
    Uniform ubo = UBO.ubo;
    PushConstant pc = PC.pc;
    ShadowPassUBO spu = SPU.spu;

    vec4 model_position = pc.object_transform * vec4(pos, 1.0);
    gl_Position = ubo.view_projection * model_position;
    fragPos = vec3(model_position);

    light_space_fragment_position = bias_matrix() * spu.view_projection * pc.object_transform * vec4(pos, 1.0);
    fragColor = colour;
    uvs = uv;
    outNormals = normals;
    outIdentifier = inIdentifier;
}
