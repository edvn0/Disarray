#define MAX_PC_IMAGE_INDICES 8

struct PushConstant {
	mat4 object_transform;
	vec4 colour;
	uint max_identifiers;
	uint max_spot_lights;
	uint max_point_lights;
	uint bound_textures;
	int image_indices[MAX_PC_IMAGE_INDICES];
};
