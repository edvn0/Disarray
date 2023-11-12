struct TransformObject {
	mat4 transform;
};

struct ColourObject {
	vec4 colour;
};


#define IDENTIFIER_OBJECT_COUNT 2000
// The idea is that we keep the last index of this for the selected one...
struct IdentifierObject {
	uint identifier;
};

struct ColourImage {
	vec4 colour;
	uint identifier;
};
