struct DirectionalLightUBO {
	vec4 position;
	vec4 direction;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 near_far; // Only using 2 components here
};
