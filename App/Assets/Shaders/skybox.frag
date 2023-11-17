#include "Buffers.glsl"

layout(location = 0) in vec3 uvs;

layout(location = 0) out vec4 colour;

void main()
{
    colour = texture(skybox_texture, uvs);
}
