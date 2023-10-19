#include "PC.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(location = 0) in vec3 position;
layout(location = 1) in uint identifier;

layout(location = 0) flat out uint out_identifier;

void main()
{
    Uniform ubo = UBO.ubo;
    PushConstant pc = PC.pc;

    vec4 model_matrix = pc.object_transform * vec4(position, 1.0);
    gl_Position = ubo.view_projection * model_matrix;
    out_identifier = identifier;
}
