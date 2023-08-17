#version 450

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

layout(location = 0) in vec4 fragColour;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;

layout(location = 0) out vec4 colour;
layout(location = 1) out uint identifier;

layout(push_constant) uniform constants
{
    mat4 object_transform;
    vec4 colour;
    uint max_identifiers;
    uint current_identifier;
} PushConstants;

void main() {
    colour = fragColour * texture(textureSampler, uvs);
    identifier = PushConstants.current_identifier;
}
