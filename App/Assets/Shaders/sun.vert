#version 450

#include "PC.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock {
	Uniform ubo;
} UBO;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 colour;
layout(location = 3) in vec3 normals;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 outNormals;

void main()
{
    Uniform ubo = UBO.ubo;

	gl_Position = ubo.view_projection * PC.object_transform * vec4(pos, 1.0);
	fragColor = PC.colour;
	uvs = uv;
	outNormals = normals;
}