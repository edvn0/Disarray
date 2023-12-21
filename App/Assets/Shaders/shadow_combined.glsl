#pragma stage vertex

#include <Buffers.glsl>
#include <MathHelpers.glsl>

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 colour;
layout(location = 3) in vec3 normals;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;

void main()
{
    mat4 vp = spu.view_projection;

    vec4 model_matrix = pc.object_transform * vec4(pos, 1.0);
    gl_Position = vp * model_matrix;
}

#pragma stage fragment

void main() { }
