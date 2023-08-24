#version 450

#include "PC.glsl"
#include "UBO.glsl"

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 colour;
layout(location = 2) in uint inIdentifier;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out flat uint outIdentifier;

void main() {
    gl_Position = UBO.view_projection * vec4(pos, 1.0);
    fragColor = colour;
    outIdentifier = inIdentifier;
}
