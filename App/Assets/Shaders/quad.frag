#version 450

#include "PC.glsl"
#include "UBO.glsl"

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 outNormals;
layout(location = 3) in flat uint identifer;

layout(location = 0) out vec4 colour;
layout(location = 1) out uint id;

void main() {
    vec3 sun_direction = vec3(UBO.sun_direction_and_intensity);
    float sun_intensity = float(UBO.sun_direction_and_intensity.w);
    vec4 sun_colour =UBO.sun_colour;

    vec4 ambient = sun_intensity * fragColor;

    float diff = max(dot(outNormals, sun_direction), 0.0);
    vec4 diffuse = diff * sun_colour;

    colour = (ambient + diffuse) * fragColor;
    id = identifer;
}
