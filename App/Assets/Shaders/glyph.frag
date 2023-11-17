#include "Buffers.glsl"

layout(location = 0) in vec2 in_tex_coords;
layout(location = 1) in vec4 colour;
layout(location = 2) in flat uint font_image_identifier;

layout(location = 0) out vec4 color;

void main()
{
    float sampled_from_text_texture = texture(sampler2D(glyph_textures[font_image_identifier], glyph_texture_sampler), in_tex_coords).r;
    vec4 sampled = vec4(1.0, 1.0, 1.0, sampled_from_text_texture);
    if (sampled.a < 0.4) discard;
    color = colour * sampled;
}
