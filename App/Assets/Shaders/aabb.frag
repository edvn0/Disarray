#include "CameraUBO.glsl"
#include "DirectionalLightUBO.glsl"
#include "ImageIndices.glsl"
#include "LightingUtilities.glsl"
#include "PC.glsl"
#include "PointLight.glsl"
#include "ShadowPassUBO.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 1) uniform CameraBlock { CameraUBO camera; }
CBO;

layout(set = 0, binding = 2) uniform PointLightBlock { PointLight[MAX_POINT_LIGHTS] lights; }
PLBO;

layout(set = 0, binding = 3) uniform ShadowPassBlock { ShadowPassUBO spu; }
SPU;

layout(set = 0, binding = 4) uniform DirectionalLightBlock { DirectionalLightUBO dlu; }
DLU;

layout(set = 1, binding = 1) uniform sampler2D depth_texture;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(location = 0) flat in vec4 fragColor;
layout(location = 1) in vec2 uvs;
layout(location = 2) flat in vec3 normals;
layout(location = 3) in vec3 fragment_position;
layout(location = 4) in vec4 light_space_fragment_position;

layout(location = 0) out vec4 colour;

void main()
{
	Uniform ubo = UBO.ubo;
	DirectionalLightUBO dlu = DLU.dlu;
	PushConstant pc = PC.pc;
	ShadowPassUBO spu = SPU.spu;

	colour = pc.colour;
}
