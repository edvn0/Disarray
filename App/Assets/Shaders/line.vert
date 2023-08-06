#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 colour;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform Uniform {
    mat4 view;
    mat4 proj;
    mat4 view_projection;
} UBO;

layout(push_constant) uniform constants
{
	mat4 object_transform;
	vec4 colour;
} PushConstants;

void main() {
    gl_Position = UBO.view_projection * vec4(pos, 1.0);
    fragColor = colour;
}
