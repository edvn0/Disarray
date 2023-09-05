#version 460
#extension GL_GOOGLE_include_directive:require

#include "PC.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock {
	Uniform ubo;
} UBO;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 colour;
layout(location = 2) in uint inIdentifier;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out flat uint outIdentifier;

void main() {
	Uniform ubo = UBO.ubo;

	gl_Position = ubo.view_projection * vec4(pos, 1.0);
	fragColor = colour;
	outIdentifier = inIdentifier;
}
