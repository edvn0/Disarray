#include "CameraUBO.glsl"
#include "DirectionalLightUBO.glsl"
#include "ImageIndices.glsl"
#include "LightingUtilities.glsl"
#include "PC.glsl"
#include "PointLight.glsl"
#include "ShadowPassUBO.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 1) uniform CameraBlock { CameraUBO camera; }
CBO;

layout(set = 0, binding = 2) uniform PointLightBlock { PointLight[MAX_POINT_LIGHTS] lights; }
PLBO;

layout(set = 0, binding = 3) uniform ShadowPassBlock { ShadowPassUBO spu; }
SPU;

layout(set = 0, binding = 4) uniform DirectionalLightBlock { DirectionalLightUBO dlu; }
DLU;

layout(set = 1, binding = 1) uniform sampler2D depth_texture;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(location = 0) flat in vec4 fragColor;
layout(location = 1) in vec2 uvs;
layout(location = 2) flat in vec3 normals;
layout(location = 3) in vec3 fragment_position;
layout(location = 4) in vec4 light_space_fragment_position;

layout(location = 0) out vec4 colour;
layout(location = 1) out uint id;

void main()
{
    Uniform ubo = UBO.ubo;
    DirectionalLightUBO dlu = DLU.dlu;
    PushConstant pc = PC.pc;
    ShadowPassUBO spu = SPU.spu;

    vec3 view_direction = normalize(vec3(CBO.camera.position) - fragment_position);

    float shadow_bias = max(0.05 * (1.0 - dot(normals, vec3(dlu.direction))), 0.005);
    float shadow = shadow_calculation(light_space_fragment_position, depth_texture, true, shadow_bias);

    DirectionalLight light;
    light.direction = vec3(dlu.direction);
    light.ambient = vec3(dlu.ambient);
    light.diffuse = vec3(dlu.diffuse);
    light.specular = vec3(dlu.specular);
    vec3 out_vec = calculate_directional_light(light, normals, view_direction, shadow, 32);

    for (uint i = 0; i < pc.max_point_lights; i++) {
        PointLight current_point_light = PLBO.lights[i];
        vec4 point_light_position = current_point_light.position;
        vec4 point_light_factors = current_point_light.factors;
        vec4 point_light_ambient = current_point_light.ambient;
        vec4 point_light_diffuse = current_point_light.diffuse;
        vec4 point_light_specular = current_point_light.specular;
        out_vec += calculate_point_light(point_light_position, point_light_factors, point_light_ambient, point_light_diffuse, point_light_specular,
        normals, fragment_position, shadow, view_direction);
    }

    colour = pc.colour * vec4(out_vec, 1.0f);
    id = pc.current_identifier;
}
