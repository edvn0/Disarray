struct SpotLight {
    vec4 position;
    vec4 direction_and_offset;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 factors;
};

#define MAX_SPOT_LIGHTS 650
