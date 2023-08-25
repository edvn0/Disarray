#version 450

#include "PC.glsl"
#include "UBO.glsl"

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 colour;
layout(location = 3) in vec3 normals;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 outNormals;

void main()
{
	gl_Position = UBO.view_projection * PushConstants.object_transform * vec4(pos, 1.0);
	fragColor = PushConstants.colour;
	uvs = uv;
	outNormals = normals;
}
