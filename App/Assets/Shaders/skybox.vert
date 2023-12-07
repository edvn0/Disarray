#include "Buffers.glsl"

layout(location = 0) in vec3 pos;

layout(location = 0) out vec3 uvs;

void main()
{
    vec4 position = ubo.proj * mat4(mat3(ubo.view)) * vec4(pos, 1.0);
    gl_Position = position.xyww;
    uvs = pos;
    uvs.xy *= -1.0;
}
