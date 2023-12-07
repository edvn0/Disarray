#include "Buffers.glsl"

layout(location = 0) in vec2 in_uvs;

layout(location = 0) out vec4 colour;

void main()
{
    vec4 scene_colour = texture(geometry_texture, in_uvs);
    vec4 font_colour = texture(font_texture, in_uvs);

    colour = font_colour * font_colour.a + scene_colour * (1 - font_colour.a);
    if (colour.a < 0.1) {
        discard;
    }
}
