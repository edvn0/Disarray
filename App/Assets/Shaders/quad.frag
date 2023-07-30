#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec2 outNormals;

layout(location = 0) out vec4 colour;

void main() {
    colour = fragColor;
}
