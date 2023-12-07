#include "MathHelpers.glsl"
#include "Buffers.glsl"

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normals;
layout(location = 3) in vec4 colour;

layout(location = 0) out vec4 fragment_colour;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 output_normals;
layout(location = 3) out vec3 fragment_position;
layout(location = 4) out vec4 light_space_fragment_position;

void main()
{
    vec4 model_position = vec4(pos, 1.0);
    gl_Position = ubo.view_projection * model_position;
    fragment_position = vec3(model_position);

    light_space_fragment_position = bias_matrix() * spu.view_projection * model_position;
    fragment_colour = colour;
    uvs = uv;
    output_normals = normals;
}
