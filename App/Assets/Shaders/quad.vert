#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec2 normals;
layout(location = 3) in vec4 colour;
layout(location = 4) in uint inIdentifier;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec2 outNormals;
layout(location = 3) out flat uint outIdentifier;

layout(set = 0, binding = 0) uniform Uniform {
    mat4 view;
    mat4 proj;
    mat4 view_projection;
} UBO;

layout(push_constant) uniform constants
{
    mat4 object_transform;
    vec4 colour;
    uint max_identifiers;
    uint current_identifier;
} PushConstants;

void main() {
    gl_Position = UBO.view_projection * vec4(pos, 1.0);
    fragColor = colour;
    uvs = uv;
    outNormals = normals;
    outIdentifier = inIdentifier;
}
