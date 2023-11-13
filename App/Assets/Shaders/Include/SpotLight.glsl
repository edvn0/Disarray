struct SpotLight {
	vec4 position;
	vec4 direction_and_cutoff;
	vec4 factors_and_outer_cutoff;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

#define MAX_SPOT_LIGHTS 650
