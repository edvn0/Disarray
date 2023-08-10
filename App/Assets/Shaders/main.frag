#version 450

layout(set = 0, binding = 1) uniform sampler2D textureSampler;

layout(location = 0) in vec4 fragColour;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;

layout(location = 0) out vec4 colour;

void main() {
    colour = texture(textureSampler, uvs);

    if (colour.a == 0.0) {
        discard;
    }
}
