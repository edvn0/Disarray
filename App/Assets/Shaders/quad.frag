#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec2 outNormals;
layout(location = 3) in flat uint identifer;

layout(location = 0) out vec4 colour;
layout(location = 1) out uint id;

layout(push_constant) uniform constants
{
    mat4 object_transform;
    vec4 colour;
    uint max_identifiers;
} PushConstants;

void main() {
    float identity = float(identifer + 1);
    float scaled_identity = identity / float(PushConstants.max_identifiers);
    colour = fragColor * vec4(scaled_identity, scaled_identity, scaled_identity, 1);
    id = identifer;
}
