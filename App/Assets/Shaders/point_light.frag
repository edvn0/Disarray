#version 450

#include "PC.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock {
    Uniform ubo;
} UBO;

layout(push_constant) uniform PushConstantBlock
{
    PushConstant pc;
}
PC;

layout(location = 0) in vec4 fragColour;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;

layout(location = 0) out vec4 colour;
layout(location = 1) out uint identifier;

void main() {
    PushConstant pc = PC.pc;

    colour = fragColour;
    identifier = pc.current_identifier;
}