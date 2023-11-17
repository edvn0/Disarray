#include "MathHelpers.glsl"
#include "Buffers.glsl"

#include "DefaultInput.glsl"

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 outNormals;

void main()
{
    mat4 vp = spu.view_projection;
    vec4 model_matrix = pc.object_transform * vec4(pos, 1.0);
    gl_Position = vp * model_matrix;
    fragColor = pc.colour;
    uvs = uv;
    outNormals = correct_normals(pc.object_transform, normals);
}
