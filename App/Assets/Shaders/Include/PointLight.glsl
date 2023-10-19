struct PointLight {
	vec4 position;
	vec4 factors;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

#define MAX_POINT_LIGHTS 1000
