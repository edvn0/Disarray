#pragma stage vertex

#include "MathHelpers.glsl"
#include "Buffers.glsl"

#include "DefaultInput.glsl"

layout(location = 0) out vec4 fragment_colour;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 out_normals;
layout(location = 3) out vec3 frag_pos;
layout(location = 4) out vec4 light_space_frag_pos;

void main()
{
    vec4 model_position = pc.object_transform * vec4(pos, 1.0);

    frag_pos = vec3(model_position);
    gl_Position = ubo.view_projection * model_position;
    light_space_frag_pos = bias_matrix() * spu.view_projection * model_position;
    fragment_colour = colour;
    uvs = uv;
    out_normals = correct_normals(pc.object_transform, normals);
}

#pragma stage fragment

#include "Buffers.glsl"
#include "Random.glsl"
#include "LightingUtilities.glsl"

layout(constant_id = 0) const int POINT_LIGHT_CHOICE = 0;
layout(constant_id = 1) const int GAMMA_CORRECT = 0;

layout(location = 0) in vec4 fragment_colour;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;
layout(location = 3) in vec3 fragment_position;
layout(location = 4) in vec4 light_space_fragment_position;

layout(location = 0) out vec4 colour;

void main()
{
    vec3 view_direction = normalize(vec3(cbo.position) - fragment_position);
    float shadow_bias = max(0.01 * (1.0 - dot(normals, vec3(dlu.direction))), 0.005);
    float shadow = shadow_calculation(light_space_fragment_position, depth_texture, true, shadow_bias);

    DirectionalLight light;
    light.direction = vec3(dlu.direction);
    light.ambient = dlu.ambient;
    light.diffuse = vec3(dlu.diffuse);
    light.specular = vec3(dlu.specular);
    vec3 out_vec = calculate_directional_light(light, uvs, normals, view_direction, shadow, 32);

    if (POINT_LIGHT_CHOICE == 0) {
        for (uint i = 0; i < plbo.max_point_lights.x; i++) {
            PointLight current_point_light = plbo.lights[i];
            vec4 point_light_position = current_point_light.position;
            vec4 point_light_factors = current_point_light.factors;
            vec4 point_light_ambient = current_point_light.ambient;
            vec4 point_light_diffuse = current_point_light.diffuse;
            vec4 point_light_specular = current_point_light.specular;
            out_vec += calculate_point_light(point_light_position, uvs, point_light_factors, point_light_ambient, point_light_diffuse,
            point_light_specular, normals, fragment_position, shadow, view_direction);
        }
    } else {
        uint base = tea(103, 107);
        uint count_max = min(8, plbo.max_point_lights.x);
        [[unroll]] for (uint i = 0; i < count_max; i++) {
            PointLight current_point_light = plbo.lights[next_uint(base, plbo.max_point_lights.x)];
            vec4 point_light_position = current_point_light.position;
            vec4 point_light_factors = current_point_light.factors;
            vec4 point_light_ambient = current_point_light.ambient;
            vec4 point_light_diffuse = current_point_light.diffuse;
            vec4 point_light_specular = current_point_light.specular;
            out_vec += calculate_point_light(point_light_position, uvs, point_light_factors, point_light_ambient, point_light_diffuse,
            point_light_specular, normals, fragment_position, shadow, view_direction);
        }
    }

    for (uint i = 0; i < slbo.max_spot_lights.x; i++) {
        SpotLight current_spot_light = slbo.lights[i];
        vec4 sl_position = current_spot_light.position;
        vec4 sl_factors = current_spot_light.factors_and_outer_cutoff;
        vec4 sl_ambient = current_spot_light.ambient;
        vec4 sl_diffuse = current_spot_light.diffuse;
        vec4 sl_specular = current_spot_light.specular;
        vec4 direction_and_cutoff = current_spot_light.direction_and_cutoff;

        vec3 sl_direction = direction_and_cutoff.xyz;
        float sl_cutoff = direction_and_cutoff.w;
        float sl_outer_cutoff = sl_factors.w;

        out_vec += calculate_spot_light(sl_position, uvs, sl_factors, sl_ambient, sl_diffuse, sl_specular, normals, fragment_position, shadow,
        view_direction, sl_direction, sl_cutoff, sl_outer_cutoff);
    }

    colour = vec4(pc.albedo_colour, 1.0F) * vec4(out_vec, 1.0F);
    if (GAMMA_CORRECT == 0) {
        colour = gamma_correct(colour);
    }
}
