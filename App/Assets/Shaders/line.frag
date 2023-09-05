#version 460
#extension GL_GOOGLE_include_directive:require

#include "PC.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock {
	Uniform ubo;
} UBO;

layout(location = 0) in vec4 fragColor;

layout(location = 0) out vec4 colour;

void main() {
	colour = fragColor;
}
