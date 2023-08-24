layout(set = 0, binding = 0) uniform Uniform
{
	mat4 view;
	mat4 proj;
	mat4 view_projection;
	vec4 sun_direction_and_intensity;
	vec4 sun_colour;
}
UBO;
