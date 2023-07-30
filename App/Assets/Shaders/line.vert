#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 colour;

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform constants
{
	mat4 object_transform;
	vec4 colour;
} PushConstants;

void main() {
    gl_Position = vec4(pos, 1.0);
    fragColor = colour;
}