#pragma stage vertex

layout(push_constant) uniform PushConstantBlock
{
    mat4 object_transform;
    vec3 albedo_colour;
    float metalness;
    float roughness;
    float emission;
    bool use_normal_map;
}
pc;

layout(set = 0, binding = 0) uniform UniformBlock {
    mat4 view;
    mat4 proj;
    mat4 view_projection;
}
ubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 colour;
layout(location = 3) in vec3 normals;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;

layout(location = 0) out vec2 frag_uv;
layout(location = 1) out vec4 frag_colour;
layout(location = 2) out vec3 frag_normals;
layout(location = 3) out vec3 frag_tangent;
layout(location = 4) out vec3 frag_bitangent;

void main() {
    gl_Position = ubo.view_projection * pc.object_transform * vec4(pos, 1.0);
    frag_uv = uv;
    frag_colour = colour;
    frag_normals = normals;
    frag_tangent = tangent;
    frag_bitangent = bitangent;
}

#pragma stage fragment

layout(push_constant) uniform PushConstantBlock
{
    mat4 object_transform;
    vec3 albedo_colour;
    float metalness;
    float roughness;
    float emission;
    bool use_normal_map;
}
pc;

layout(location = 0) in vec2 frag_uv;
layout(location = 1) in vec4 frag_colour;
layout(location = 2) in vec3 frag_normals;
layout(location = 3) in vec3 frag_tangent;
layout(location = 4) in vec3 frag_bitangent;

layout(location = 0) out vec4 out_colour;

layout(binding = 16) uniform sampler2D albedo_map;
layout(binding = 17) uniform sampler2D diffuse_map;
layout(binding = 18) uniform sampler2D specular_map;
layout(binding = 19) uniform sampler2D normal_map;
layout(binding = 20) uniform sampler2D metalness_map;
layout(binding = 21) uniform sampler2D roughness_map;

void main() {
    vec3 albedo = texture(albedo_map, frag_uv).rgb * pc.albedo_colour;
    out_colour = vec4(albedo, 1.0f);
}
