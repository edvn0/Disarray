#include "GlyphUBO.glsl"
#include "PC.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 5) uniform GlyphUBOBlock { GlyphUBO gbo; }
GBO;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(set = 2, binding = 2) uniform texture2D glyph_textures[128];
layout(set = 2, binding = 3) uniform sampler glyph_texture_sampler;

layout(location = 0) in vec2 in_tex_coords;
layout(location = 0) out vec4 color;

void main()
{
    PushConstant pc = PC.pc;
    float sampled_from_text_texture = texture(sampler2D(glyph_textures[pc.image_indices[0]], glyph_texture_sampler), in_tex_coords).r;
    vec4 sampled = vec4(1.0, 1.0, 1.0, sampled_from_text_texture);
    if (sampled.a < 0.1) discard;
    color = pc.colour * sampled;
}
