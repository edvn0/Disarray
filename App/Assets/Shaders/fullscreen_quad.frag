layout(location = 0) in vec2 in_uvs;

layout(location = 0) out vec4 colour;

layout(set = 1, binding = 0) uniform sampler2D geometry_texture;
layout(set = 1, binding = 2) uniform sampler2D font_texture;

void main()
{
	vec4 scene_colour = texture(geometry_texture, in_uvs);
	vec4 font_colour = texture(font_texture, in_uvs);

	colour = font_colour * font_colour.a + scene_colour * (1 - font_colour.a);
	if (colour.a < 0.1)
		discard;
}
