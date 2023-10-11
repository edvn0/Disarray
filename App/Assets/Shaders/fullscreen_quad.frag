layout(location = 0) in vec2 in_uvs;

layout(location = 0) out vec4 colour;

layout(set = 1, binding = 0) uniform sampler2D geometry_texture;
layout(set = 1, binding = 2) uniform sampler2D font_texture;

void main() {
    vec4 geometry = texture(geometry_texture, in_uvs);
    vec4 font = texture(font_texture, in_uvs);



    colour = geometry + font;
}
