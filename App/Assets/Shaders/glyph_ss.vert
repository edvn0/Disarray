#include "Buffers.glsl"

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_tex_coords;

layout(location = 0) out vec2 out_tex_coords;
layout(location = 1) out vec4 colour;
layout(location = 2) out flat uint font_image_identifier;

void main()
{
    gl_Position = gbo.projection * vec4(in_pos, 0.0, 1.0);
    out_tex_coords = in_tex_coords;
    uint index = uint(gl_VertexIndex / 4);
    colour = FontColourImageSSBO.ssbo_objects[index].colour;
    font_image_identifier = FontColourImageSSBO.ssbo_objects[index].identifier;
}
