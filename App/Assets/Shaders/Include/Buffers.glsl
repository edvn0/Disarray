layout(push_constant) uniform PushConstantBlock
{
    mat4 object_transform;
    vec3 albedo_colour;
    float metalness;
    float roughness;
    float emission;
    float diffuse_factor;
    bool use_normal_map;
}
pc;

layout(binding = 0) uniform UniformBlock {
    mat4 view;
    mat4 proj;
    mat4 view_projection;
}
ubo;

layout(binding = 1) uniform CameraUBO {
    vec4 position;
    vec4 direction;
    mat4 view;
}
cbo;

layout(binding = 2) uniform ShadowPassUBO {
    mat4 view;
    mat4 proj;
    mat4 view_projection;
}
spu;

layout(binding = 3) uniform DirectionalLightUBO {
    vec4 position;
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 near_far;// Only using 2 components here
}
dlu;

struct PointLight {
    vec4 position;
    vec4 factors;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};
#define MAX_POINT_LIGHTS 800

layout(std140, binding = 4) uniform PointLightUBO {
    uint count;
    PointLight lights[MAX_POINT_LIGHTS];
}
PLBO;

struct SpotLight {
    vec4 position;
    vec4 direction_and_cutoff;
    vec4 factors_and_outer_cutoff;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};
#define MAX_SPOT_LIGHTS 650

layout(std140, binding = 5) uniform SpotLightUBO {
    uint count;
    SpotLight lights[MAX_SPOT_LIGHTS];
}
SLBO;


struct IdentifierObject {
    uint identifier;
};
layout(std430, binding = 6) readonly buffer Identifiers { IdentifierObject ssbo_objects[]; }
IdentifierSSBO;

struct TransformObject {
    mat4 transform;
};
layout(std140, binding = 7) readonly buffer Transforms { TransformObject ssbo_objects[]; }
TransformSSBO;
