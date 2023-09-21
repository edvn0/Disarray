#include "PC.glsl"
#include "UBO.glsl"
#include "PointLight.glsl"
#include "CameraUBO.glsl"
#include "ImageIndices.glsl"

layout(set = 0, binding = 0) uniform UniformBlock {
    Uniform ubo;
} UBO;

layout(set = 0, binding = 1) uniform CameraBlock {
    CameraUBO camera;
} CBO;

layout(set = 0, binding = 2) uniform PointLightBlock {
    PointLight[MAX_POINT_LIGHTS] lights;
} PLBO;

layout(push_constant) uniform PushConstantBlock
{
    PushConstant pc;
}
PC;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 outNormals;
layout(location = 3) in vec3 fragPosition;

layout(location = 0) out vec4 colour;
layout(location = 1) out uint id;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec3 calc_dir_light(DirLight light, vec3 normal, vec3 view_direction, uint spec_pow);
vec3 calc_point_light(PointLight light, vec3 normal, vec3 fragPos, vec3 view_direction);


void main() {
    Uniform ubo = UBO.ubo;
    PushConstant pc = PC.pc;

    vec3 out_vec = vec3(0.0);
    vec3 view_direction = normalize(vec3(CBO.camera.position) - fragPosition);

    DirLight light;
    light.direction = vec3(ubo.sun_direction_and_intensity);
    light.ambient = vec3(ubo.sun_colour);
    light.diffuse = vec3(0.9);
    light.specular = vec3(0.9);
    out_vec += vec3(pc.colour) * calc_dir_light(light, outNormals, view_direction, 32);
    for (uint i = 0; i < pc.max_point_lights; i++) {
        PointLight light = PLBO.lights[i];
        out_vec += calc_point_light(light, outNormals, fragPosition, view_direction);
    }

    colour = pc.colour * vec4(out_vec, 1.0f);
    id = pc.current_identifier;
}

vec3 calc_dir_light(DirLight light, vec3 normal, vec3 view_direction, uint spec_pow)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(lightDir, normal);
    float spec = pow(max(dot(view_direction, reflectDir), 0.0), spec_pow);
    // combine results
    vec3 ambient  = light.ambient;
    vec3 diffuse  = light.diffuse  * diff;
    vec3 specular = light.specular * spec;
    return (ambient + diffuse + specular);
}

vec3 calc_point_light(PointLight light, vec3 normal, vec3 fragPos, vec3 view_direction) {
    vec3 light_to_frag_vector = vec3(light.position) - fragPos;
    vec3 lightDir = normalize(light_to_frag_vector);
    // diffuse shading
    float diff = max(dot(normal, vec3(lightDir)), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(view_direction, reflectDir), 0.0), 32);
    // attenuation
    float distance    = length(light_to_frag_vector);

    float factors = dot(vec3(light.factors), vec3(1, distance, distance * distance));
    float attenuation = 1.0F / max(0.5, factors);

    // combine results
    vec4 ambient  = light.ambient;
    vec4 diffuse  = light.diffuse  * diff;
    vec4 specular = light.specular * spec;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return vec3(ambient + diffuse + specular);
}
