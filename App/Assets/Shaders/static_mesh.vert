#include "MathHelpers.glsl"
#include "Buffers.glsl"

#include "DefaultInput.glsl"

layout(location = 0) out vec4 fragment_colour;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 out_normals;
layout(location = 3) out vec3 frag_pos;
layout(location = 4) out vec4 light_space_frag_pos;

void main()
{
    vec4 model_position = pc.object_transform * vec4(pos, 1.0);

    frag_pos = vec3(model_position);
    gl_Position = ubo.view_projection * model_position;
    light_space_frag_pos = bias_matrix() * spu.view_projection * model_position;
    fragment_colour = colour;
    uvs = uv;
    out_normals = correct_normals(pc.object_transform, normals);
}
