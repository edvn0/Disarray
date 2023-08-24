layout(push_constant) uniform constants
{
	mat4 object_transform;
	vec4 colour;
	uint max_identifiers;
	uint current_identifier;
}
PushConstants;
