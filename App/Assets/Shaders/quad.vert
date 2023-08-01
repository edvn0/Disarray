#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec2 normals;
layout(location = 3) in vec4 colour;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec2 outNormals;

layout(push_constant) uniform constants
{
	mat4 object_transform;
	vec4 colour;
} PushConstants;

void main() {
    gl_Position = vec4(pos, 1.0);
    fragColor = colour;
    uvs = uv;
    outNormals = normals;
}