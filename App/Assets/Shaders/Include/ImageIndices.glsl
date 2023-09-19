#define MAX_IMAGE_INDICES 64

struct ImageIndices {
	uint bound_textures;
	uvec4 image_indices[MAX_IMAGE_INDICES];
};
