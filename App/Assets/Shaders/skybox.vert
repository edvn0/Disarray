#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout (location = 0) in vec3 pos;

layout(location = 0) out vec3 uvs;

void main()
{
    vec4 fragment_position = UBO.ubo.view_projection * vec4(pos, 1.0);
    gl_Position = fragment_position.xyww;
    uvs = pos;
}
