#include "GlyphUBO.glsl"
#include "PC.glsl"
#include "UBO.glsl"

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_tex_coords;

layout(location = 0) out vec2 out_tex_coords;

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 5) uniform GlyphUBOBlock { GlyphUBO gbo; }
GBO;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

void main()
{
	GlyphUBO gbo = GBO.gbo;

	gl_Position = gbo.projection * vec4(in_pos, 0.0, 1.0);
	out_tex_coords = in_tex_coords;
}
