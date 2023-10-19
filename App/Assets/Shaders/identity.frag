#include "PC.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(location = 0) flat in uint identifier;

layout(location = 0) out uint out_identifier;

void main()
{
    Uniform ubo = UBO.ubo;
    PushConstant pc = PC.pc;

    out_identifier = identifier;
}
