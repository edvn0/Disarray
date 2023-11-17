#include "Buffers.glsl"
#include "Random.glsl"
#include "LightingUtilities.glsl"

layout(constant_id = 0) const int POINT_LIGHT_CHOICE = 0;

layout(location = 0) in vec4 fragment_colour;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;
layout(location = 3) in vec3 fragment_position;
layout(location = 4) in vec4 light_space_fragment_position;

layout(location = 0) out vec4 colour;

vec3 calc_point_light(PointLight light, vec2 tex_coords, float shadow, vec3 normal, vec3 fragment_pos, vec3 view_direction)
{
    return calculate_point_light(
    light.position, tex_coords, light.factors, light.ambient, light.diffuse, light.specular, normal, fragment_pos, shadow, view_direction);
}

void main()
{
    vec3 view_direction = normalize(vec3(cbo.position) - fragment_position);

    float shadow_bias = max(0.05 * (1.0 - dot(normals, vec3(dlu.direction))), 0.005);
    float shadow = shadow_calculation(light_space_fragment_position, depth_texture, true, shadow_bias);

    DirectionalLight light;
    light.direction = vec3(dlu.direction);
    light.ambient = dlu.ambient;
    light.diffuse = vec3(dlu.diffuse);
    light.specular = vec3(dlu.specular);
    vec3 out_vec = calculate_directional_light(light, uvs, normals, view_direction, shadow, 32);

    if (POINT_LIGHT_CHOICE == 0) {
        for (uint i = 0; i < pc.max_point_lights; i++) {
            PointLight light = plbo.lights[i];
            out_vec += calc_point_light(light, uvs, shadow, normals, fragment_position, view_direction);
        }
    } else {
        uint base = tea(103, 107);
        uint count_max = min(8, pc.max_point_lights);
        [[unroll]] for (uint i = 0; i < count_max; i++) {
            PointLight light = plbo.lights[next_uint(base, pc.max_point_lights)];
            out_vec += calc_point_light(light, uvs, shadow, normals, fragment_position, view_direction);
        }
    }

    colour = fragment_colour * vec4(out_vec, 1.0f);
}
