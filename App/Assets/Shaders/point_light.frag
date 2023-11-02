#include "CameraUBO.glsl"
#include "DirectionalLightUBO.glsl"
#include "LightingUtilities.glsl"
#include "PC.glsl"
#include "UBO.glsl"

layout(constant_id = 1) const int GAMMA_CORRECT = 0;

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 1) uniform CameraBlock { CameraUBO camera; }
CBO;

layout(set = 0, binding = 4) uniform DirectionalLightBlock { DirectionalLightUBO dlu; }
DLU;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(set = 1, binding = 1) uniform sampler2D depth_texture;

layout(location = 0) in vec4 frag_colour;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;
layout(location = 3) in vec3 fragment_position;
layout(location = 4) in vec4 light_space_fragment_position;

layout(location = 0) out vec4 colour;

void main()
{
    PushConstant pc = PC.pc;
    DirectionalLightUBO dlu = DLU.dlu;

    vec3 view_direction = normalize(vec3(CBO.camera.position) - fragment_position);
    float shadow_bias = max(0.05 * (1.0 - dot(normals, vec3(dlu.direction))), 0.005);
    float shadow = shadow_calculation(light_space_fragment_position, depth_texture, true, shadow_bias);

    DirectionalLight light;
    light.direction = vec3(dlu.direction);
    light.ambient = dlu.ambient;
    light.diffuse = vec3(dlu.diffuse);
    light.specular = vec3(dlu.specular);
    vec3 out_vec = calculate_directional_light(light, normals, view_direction, shadow, 32);

    colour = frag_colour * vec4(out_vec, 1.0F);
    if (GAMMA_CORRECT == 0) {
        colour = gamma_correct(colour);
    }
}
