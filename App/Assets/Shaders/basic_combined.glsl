#pragma stage vertex

#include <Buffers.glsl>
#include <MathHelpers.glsl>

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
layout(location = 5) out vec3 frag_position;
layout(location = 6) out vec4 frag_pos_light_space;

void main() {
    vec4 model_position = pc.object_transform * vec4(pos, 1.0);
    mat4 object_matrix = TransformSSBO.ssbo_objects[gl_InstanceIndex].transform;
    uint object_identifier = IdentifierSSBO.ssbo_objects[gl_InstanceIndex].identifier;

    gl_Position = ubo.view_projection * model_position;
    frag_position = vec3(model_position);
    frag_uv = uv;
    frag_colour = colour;
    frag_normals = correct_normals(pc.object_transform, normals);
    frag_tangent = tangent;
    frag_bitangent = bitangent;
    frag_pos_light_space = bias_matrix() * spu.view_projection * model_position;
}

#pragma stage fragment

#include <Buffers.glsl>

layout(location = 0) in vec2 frag_uv;
layout(location = 1) in vec4 frag_colour;
layout(location = 2) in vec3 frag_normals;
layout(location = 3) in vec3 frag_tangent;
layout(location = 4) in vec3 frag_bitangent;
layout(location = 5) in vec3 frag_position;
layout(location = 6) in vec4 frag_pos_light_space;

layout(location = 0) out vec4 out_colour;

layout(binding = 16) uniform sampler2D albedo_map;
layout(binding = 17) uniform sampler2D diffuse_map;
layout(binding = 18) uniform sampler2D specular_map;
layout(binding = 19) uniform sampler2D normal_map;
layout(binding = 20) uniform sampler2D metalness_map;
layout(binding = 21) uniform sampler2D roughness_map;

void main() {
    vec3 albedo = texture(albedo_map, frag_uv).rgb * pc.albedo_colour;
    vec3 diffuse = texture(diffuse_map, frag_uv).rgb;
    vec3 specular = texture(specular_map, frag_uv).rgb;
    vec3 normal = texture(normal_map, frag_uv).rgb;
    float metalness = texture(metalness_map, frag_uv).r;
    float roughness = texture(roughness_map, frag_uv).r;

    // Combine all sampler2Ds please
    out_colour = vec4(albedo, 1.0f) * vec4(diffuse, 1.0f) * vec4(specular, 1.0f) * vec4(normal, 1.0f);
}
