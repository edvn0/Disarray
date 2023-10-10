#include "CameraUBO.glsl"
#include "DirectionalLightUBO.glsl"
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

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 outNormals;
layout(location = 3) in vec3 fragPosition;
layout(location = 4) in flat uint identifer;

layout(location = 0) out vec4 colour;
layout(location = 1) out uint id;

vec3 calc_point_light(PointLight light, float shadow, vec3 normal, vec3 fragPos, vec3 view_direction)
{
	return calculate_point_light(
		light.position, light.factors, light.ambient, light.diffuse, light.specular, normal, fragPos, shadow, view_direction);
}

void main()
{
	Uniform ubo = UBO.ubo;
	DirectionalLightUBO dlu = DLU.dlu;
	PushConstant pc = PC.pc;

	vec3 out_vec = vec3(0.0);
	vec3 view_direction = normalize(vec3(CBO.camera.position) - fragPosition);

	float shadow = 0.0F;

	DirectionalLight light;
	light.direction = vec3(dlu.direction);
	light.ambient = vec3(dlu.ambient);
	light.diffuse = vec3(dlu.diffuse);
	light.specular = vec3(dlu.specular);
	out_vec += calculate_directional_light(light, outNormals, view_direction, shadow, 32);
	for (uint i = 0; i < pc.max_point_lights; i++) {
		PointLight light = PLBO.lights[i];
		out_vec += calc_point_light(light, shadow, outNormals, fragPosition, view_direction);
	}

	colour = pc.colour;
	id = pc.current_identifier;
}
