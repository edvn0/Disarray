#version 450

#include "PC.glsl"
#include "UBO.glsl"

layout(location = 0) in vec4 fragColor;
layout(location = 1) in flat uint inIdentifier;

layout(location = 0) out vec4 colour;
layout(location = 1) out uint id;

void main() {
    colour = fragColor;
    id = inIdentifier;
}
