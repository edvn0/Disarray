#version 450

#include "PC.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock {
	Uniform ubo;
} UBO;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 outNormals;
layout(location = 3) in flat uint identifer;

layout(location = 0) out vec4 colour;
layout(location = 1) out uint id;

void main() {
    Uniform ubo = UBO.ubo;

    vec3 sun_direction = vec3(ubo.sun_direction_and_intensity);
    float sun_intensity = float(ubo.sun_direction_and_intensity.w);
    vec4 sun_colour =   ubo.sun_colour;

    vec4 ambient = sun_intensity * fragColor;

    float diff = max(dot(outNormals, sun_direction), 0.0);
    vec4 diffuse = diff * sun_colour;

    colour = (ambient + diffuse) * fragColor;
    id = identifer;
}
