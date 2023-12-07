#include "MathHelpers.glsl"
#include "Buffers.glsl"

#include "DefaultInput.glsl"

void main()
{
    mat4 vp = spu.view_projection;
    vec4 model_matrix = pc.object_transform * vec4(pos, 1.0);
    gl_Position = vp * model_matrix;
}
