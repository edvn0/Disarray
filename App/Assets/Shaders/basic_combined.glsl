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
#include <LightingUtilities.glsl>
#include <MathHelpers.glsl>

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
    vec3 albedo = texture(albedo_map, frag_uv).rgb;
    vec3 diffuse = texture(diffuse_map, frag_uv).rgb;
    vec3 specular = texture(specular_map, frag_uv).rgb;
    float metalness = texture(metalness_map, frag_uv).r;
    float roughness = texture(roughness_map, frag_uv).r;

    vec3 sampled_normalized_normals = vec3(0);
    if (pc.use_normal_map)
    {
        sampled_normalized_normals = normalize(texture(normal_map, frag_uv).rgb * 2.0 - 1.0);
    }
    else
    {
        sampled_normalized_normals = normalize(frag_normals);
    }

    vec3 view_direction = normalize(vec3(cbo.position) - frag_position);
    float shadow_bias = max(0.01 * (1.0 - dot(sampled_normalized_normals, vec3(dlu.direction))), 0.005);
    float shadow = 0.0;

    DirectionalLight light;
    light.direction = vec3(dlu.direction);
    light.ambient = dlu.ambient;
    light.diffuse = vec3(dlu.diffuse);
    light.specular = vec3(dlu.specular);

    vec4 sampled_albedo = texture(albedo_map, frag_uv);
    vec4 sampled_specular = texture(specular_map, frag_uv);

    vec3 out_vec = calculate_directional_light_textured(light, sampled_normalized_normals, view_direction, shadow, 32, sampled_albedo, sampled_specular);
    for (uint i = 0; i < PLBO.count; i++) {
        PointLight current_point_light = PLBO.lights[i];
        vec4 point_light_position = current_point_light.position;
        vec4 point_light_factors = current_point_light.factors;
        vec4 point_light_ambient = current_point_light.ambient;
        vec4 point_light_diffuse = current_point_light.diffuse;
        vec4 point_light_specular = current_point_light.specular;
        out_vec += calculate_point_light(point_light_position, point_light_factors, point_light_ambient, point_light_diffuse,
        point_light_specular, sampled_normalized_normals, frag_position, shadow, view_direction, sampled_albedo, sampled_specular);
    }

    for (uint i = 0; i < SLBO.count; i++) {
        SpotLight current_spot_light = SLBO.lights[i];
        vec4 sl_position = current_spot_light.position;
        vec4 sl_factors = current_spot_light.factors_and_outer_cutoff;
        vec4 sl_ambient = current_spot_light.ambient;
        vec4 sl_diffuse = current_spot_light.diffuse;
        vec4 sl_specular = current_spot_light.specular;
        vec4 direction_and_cutoff = current_spot_light.direction_and_cutoff;

        vec3 sl_direction = direction_and_cutoff.xyz;
        float sl_cutoff = direction_and_cutoff.w;
        float sl_outer_cutoff = sl_factors.w;


        out_vec += calculate_spot_light(sl_position, sl_factors, sl_ambient, sl_diffuse, sl_specular, sampled_normalized_normals, frag_position, shadow,
        view_direction, sl_direction, sl_cutoff, sl_outer_cutoff, sampled_albedo, sampled_specular);
    }

    out_colour = vec4(out_vec, 1.0);
    out_colour = gamma_correct(out_colour);
}
