#define MAX_SPOT_LIGHTS 650
#define MAX_POINT_LIGHTS 800
#define IDENTIFIER_OBJECT_COUNT 2000
#define MAX_PC_IMAGE_INDICES 8

struct PointLight {
    vec4 position;
    vec4 factors;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};
struct SpotLight {
    vec4 position;
    vec4 direction_and_cutoff;
    vec4 factors_and_outer_cutoff;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};
struct TransformObject {
    mat4 transform;
};
struct ColourObject {
    vec4 colour;
};
struct IdentifierObject {
    uint identifier;
};
struct FontColourImage {
    vec4 colour;
    uint identifier;
};

layout(push_constant) uniform PushConstantBlock {
    mat4 object_transform;
    vec4 colour;
    uint max_identifiers;
    uint max_spot_lights;
    uint max_point_lights;
    uint bound_textures;
    int image_indices[MAX_PC_IMAGE_INDICES];
}
pc;

layout(set = 0, binding = 0) uniform UniformBlock {
    mat4 view;
    mat4 proj;
    mat4 view_projection;
}
ubo;

layout(set = 0, binding = 1) uniform CameraUniformBlock {
    vec4 position;
    vec4 direction;
    mat4 view;
}
cbo;

layout(set = 0, binding = 2) uniform PointLightBlock { PointLight[MAX_POINT_LIGHTS] lights; }
plbo;

layout(set = 0, binding = 3) uniform ShadowPassBlock {
    mat4 view;
    mat4 proj;
    mat4 view_projection;
}
spu;

layout(set = 0, binding = 4) uniform DirectionalLightBlock {
    vec4 position;
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 near_far;
}
dlu;

layout(set = 0, binding = 5) uniform SpotLightBlock { SpotLight[MAX_POINT_LIGHTS] lights; }
slbo;

layout(set = 0, binding = 6) uniform GlyphUBOBlock {
    mat4 projection;
    mat4 view;
}
gbo;

layout(std140, set = 0, binding = 7) readonly buffer EntityTransformBlock { TransformObject ssbo_objects[]; }
EntityTransformSSBO;

layout(std140, set = 0, binding = 8) readonly buffer EntityIdentifierBlock { IdentifierObject ssbo_objects[]; }
EntityIdentifierSSBO;

layout(std140, set = 0, binding = 9) readonly buffer FontColourImageBlock { FontColourImage ssbo_objects[]; }
FontColourImageSSBO;

layout(set = 0, binding = 10) uniform sampler2D depth_texture;
layout(set = 0, binding = 11) uniform sampler2D geometry_texture;
layout(set = 0, binding = 12) uniform sampler2D font_texture;
layout(set = 0, binding = 13) uniform texture2D glyph_textures[128];
layout(set = 0, binding = 14) uniform sampler glyph_texture_sampler;
layout(set = 0, binding = 15) uniform samplerCube skybox_texture;
