#include "Buffers.glsl"

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 colour;

layout(location = 0) out vec4 fragColor;

void main()
{
    gl_Position = ubo.view_projection * vec4(pos, 1.0);
    fragColor = colour;
}
