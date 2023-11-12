#include "GlyphUBO.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 5) uniform GlyphUBOBlock { GlyphUBO gbo; }
GBO;

layout(set = 2, binding = 0) uniform texture2D glyph_textures[128];
layout(set = 2, binding = 1) uniform sampler glyph_texture_sampler;

layout(location = 0) in vec2 in_tex_coords;
layout(location = 1) in vec4 colour;
layout(location = 2) flat in uint font_data_identifier;

layout(location = 0) out vec4 color;

void main()
{
    float sampled_from_text_texture = texture(sampler2D(glyph_textures[font_data_identifier], glyph_texture_sampler), in_tex_coords).r;
    vec4 sampled = vec4(1.0, 1.0, 1.0, sampled_from_text_texture);
    if (sampled.a < 0.1) discard;
    color = colour * sampled;
}
