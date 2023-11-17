#include "Buffers.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex_coords;

layout(location = 0) out vec2 out_tex_coords;
layout(location = 1) out vec4 colour;
layout(location = 2) out flat uint font_image_identifier;

const float radius = 1.0F;

void main()
{
    mat3 view = mat3(cbo.view);
    vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 up = vec3(view[0][1], view[1][1], view[2][1]);

    vec3 calculated_position = in_pos - right * in_tex_coords.x - up * in_tex_coords.y;

    gl_Position = ubo.proj * cbo.view * vec4(calculated_position, 1.0F);

    out_tex_coords = in_tex_coords;
    uint index = uint(gl_VertexIndex / 4);
    colour = FontColourImageSSBO.ssbo_objects[index].colour;
    font_image_identifier = FontColourImageSSBO.ssbo_objects[index].identifier;
}
