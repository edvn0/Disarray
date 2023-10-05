#include "PC.glsl"
#include "ShadowPassUBO.glsl"
#include "UBO.glsl"

layout(set = 0, binding = 0) uniform UniformBlock { Uniform ubo; }
UBO;

layout(set = 0, binding = 3) uniform ShadowPassUniformBlock {
  ShadowPassUBO spu;
}
SPU;

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normals;
layout(location = 3) in vec4 colour;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 uvs;
layout(location = 2) out vec3 out_normals;
layout(location = 3) out vec3 frag_pos;
layout(location = 4) out vec4 light_space_frag_pos;

vec3 correct_normals(mat4 model_matrix, vec3 normals);
vec3 correct_normals(mat4 model_matrix, vec4 normals);

const mat4 bias_matrix = mat4(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0,
                              1.0, 0.0, 0.5, 0.5, 0.0, 1.0);

void main() {
  Uniform ubo = UBO.ubo;
  PushConstant pc = PC.pc;
  ShadowPassUBO spu = SPU.spu;

  vec4 model_position = pc.object_transform * vec4(pos, 1.0);

  frag_pos = vec3(model_position);
  gl_Position = ubo.view_projection * model_position;
  light_space_frag_pos = bias_matrix * spu.view_projection * model_position;
  fragColor = colour;
  uvs = uv;
  out_normals = correct_normals(pc.object_transform, normals);
}

vec3 correct_normals(mat4 model_matrix, vec3 normals) {
  return normalize(transpose(inverse(mat3(model_matrix))) * normals);
}

vec3 correct_normals(mat4 model_matrix, vec4 normals) {
  return correct_normals(model_matrix, vec3(normals));
}
