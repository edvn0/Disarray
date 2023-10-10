#include "PC.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normals;
layout(location = 3) in vec4 colour;
layout(location = 4) in uint inIdentifier;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 outNormals;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out flat uint outIdentifier;

void main()
{
	Uniform ubo = UBO.ubo;

	gl_Position = ubo.view_projection * vec4(pos, 1.0);
	fragPos = pos;
	fragColor = colour;
	uvs = uv;
	outNormals = normals;
	outIdentifier = inIdentifier;
}
