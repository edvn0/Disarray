#include "UBO.glsl"
#include "GlyphUBO.glsl"
#include "PC.glsl"

layout(set = 0, binding = 0) uniform UniformBlock {
    Uniform ubo;
} UBO;

layout(set = 0, binding = 5) uniform GlyphUBOBlock {
    GlyphUBO gbo;
} GBO;

layout(push_constant) uniform PushConstantBlock
{
    PushConstant pc;
}
PC;

layout(set = 2, binding = 2) uniform sampler2D textures[127];

layout(location = 0) in vec2 in_tex_coords;
layout(location = 0) out vec4 color;

void main()
{
    PushConstant pc = PC.pc;
    int chosen_texture_to_sample = pc.image_indices[0];
    float sampled_from_text_texture = texture(textures[chosen_texture_to_sample], in_tex_coords).r;
    vec4 sampled = vec4(1.0, 1.0, 1.0, sampled_from_text_texture);
    color = pc.colour * sampled;
}
