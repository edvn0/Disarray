#include "CameraUBO.glsl"
#include "DirectionalLightUBO.glsl"
#include "ImageIndices.glsl"
#include "LightingUtilities.glsl"
#include "PC.glsl"
#include "PointLight.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 1) uniform CameraBlock { CameraUBO camera; }
CBO;

layout(set = 0, binding = 2) uniform PointLightBlock { PointLight[MAX_POINT_LIGHTS] lights; }
PLBO;

layout(set = 0, binding = 4) uniform DirectionalLightBlock { DirectionalLightUBO dlu; }
DLU;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(set = 2, binding = 0) uniform texture2D textures[46];
layout(set = 2, binding = 1) uniform sampler texture_sampler;

layout(set = 1, binding = 1) uniform sampler2D depth_texture;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;
layout(location = 3) in vec3 fragment_position;
layout(location = 4) in vec4 light_space_fragment_position;

layout(location = 0) out vec4 colour;
layout(location = 1) out uint id;

vec3 calc_point_light(PointLight light, float shadow, vec3 normal, vec3 frag_pos, vec3 view_direction)
{
	return calculate_point_light(
		light.position, light.factors, light.ambient, light.diffuse, light.specular, normal, frag_pos, shadow, view_direction);
}

void main()
{
	Uniform ubo = UBO.ubo;
	PushConstant pc = PC.pc;
	DirectionalLightUBO dlu = DLU.dlu;

	vec3 view_direction = normalize(vec3(CBO.camera.position) - fragment_position);
	float shadow_bias = max(0.05 * (1.0 - dot(normals, vec3(dlu.direction))), 0.005);
	float shadow = shadow_calculation(light_space_fragment_position, depth_texture, false, shadow_bias);

	DirectionalLight light;
	light.direction = vec3(dlu.direction);
	light.ambient = vec3(dlu.ambient);
	light.diffuse = vec3(dlu.diffuse);
	light.specular = vec3(dlu.specular);
	vec3 out_vec = calculate_directional_light(light, normals, view_direction, shadow, 32);
	for (uint i = 0; i < pc.max_point_lights; i++) {
		PointLight light = PLBO.lights[i];
		out_vec += calc_point_light(light, shadow, normals, fragment_position, view_direction);
	}

	vec4 mean_texture_colour = vec4(0.0);
	for (uint bound_image_index = 0; bound_image_index < pc.bound_textures; bound_image_index++) {
		int index = pc.image_indices[bound_image_index];
		mean_texture_colour += texture(sampler2D(textures[index], texture_sampler), uvs);
	}

	colour = pc.colour;
	if (pc.bound_textures > 0) {
		colour *= mean_texture_colour;
	}

	if (colour.a < 0.1)
		discard;

	colour *= vec4(out_vec, 1.0f);
	colour = gamma_correct(colour);
	id = pc.current_identifier;
}
