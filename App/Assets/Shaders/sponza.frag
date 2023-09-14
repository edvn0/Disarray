#include "PC.glsl"
#include "UBO.glsl"
#include "PointLight.glsl"
#include "CameraUBO.glsl"

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

vec3 calc_dir_light(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	// combine results
	vec3 ambient  = light.ambient;
	vec3 diffuse  = light.diffuse  * diff;
	vec3 specular = light.specular * spec;
	return (ambient + diffuse + specular);
}

vec3 calc_point_light(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	vec3 lightDir = normalize(vec3(light.position) - fragPos);
	// diffuse shading
	float diff = max(dot(normal, vec3(lightDir)), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	// attenuation
	float distance    = length(vec3(light.position) - fragPos);

	float constant = light.factors.x;
	float lin = light.factors.y;
	float quadratic = light.factors.z;
	float attenuation = 1.0 / (constant + lin * distance + quadratic * (distance * distance));
	// combine results
	vec3 ambient  = vec3(light.ambient);
	vec3 diffuse  = vec3(light.diffuse)  * diff;
	vec3 specular = vec3(light.specular) * spec;
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;
	return (ambient + diffuse + specular);
}


void main() {
	Uniform ubo = UBO.ubo;
  PushConstant pc = PC.pc;

	vec3 out_vec = vec3(0.0);
	vec3 viewDir = normalize(vec3(CBO.camera.position) - fragPosition);

	DirLight light;
	light.direction = vec3(ubo.sun_direction_and_intensity);
	light.ambient = vec3(ubo.sun_colour);
	light.diffuse = vec3(0.1, 0.9, 0.9);
	light.specular = vec3(0.1, 0.9, 0.9);
	out_vec += calc_dir_light(light, outNormals, viewDir);
	for (uint i = 0; i < 1; i++) {
		PointLight light = PLBO.lights[i];
		out_vec += calc_point_light(light, outNormals, fragPosition, viewDir);
	}

	//colour = pc.colour * vec4(out_vec, 1.0f);
  colour = vec4(pc.max_point_lights, 1.0f, 1.0f, 1.0f);

	id = pc.current_identifier;
}