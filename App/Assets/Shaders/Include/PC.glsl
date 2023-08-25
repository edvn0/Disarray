layout(push_constant) uniform PushConstantBlock
{
	mat4 object_transform;
	vec4 colour;
	uint max_identifiers;
	uint current_identifier;
	uint max_point_lights;
}
PC;
