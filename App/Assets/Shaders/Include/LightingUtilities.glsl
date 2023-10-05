struct DirectionalLight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

#define SHADOWED_AMBIENT_LIGHT 0.1

vec3 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 view_direction, float shadow, uint spec_pow);

vec3 calculate_point_light(
	vec4 position, vec4 factors, vec4 ambient, vec4 diffuse, vec4 specular, vec3 normal, vec3 fragPos, float shadow, vec3 view_direction);
float shadow_calculation(vec4 view_projection, sampler2D depth_texture, bool use_pdf, float bias);
float shadow_calculation(vec4 view_projection, sampler2D depth_texture, bool use_pdf)
{
	return shadow_calculation(view_projection, depth_texture, false, 0.005);
}
float shadow_calculation(vec4 view_projection, sampler2D depth_texture) { return shadow_calculation(view_projection, depth_texture, false, 0.005); }
vec4 gamma_correct(vec4 colour);

float shadow_calculation(vec4 view_projection, sampler2D depth_texture, bool use_pdf, float shadow_bias)
{
	vec3 projection_coordinates = view_projection.xyz / view_projection.w;

	if (view_projection.w <= 0) {
		return SHADOWED_AMBIENT_LIGHT;
	}

	const float shadow_map_closest_depth = texture(depth_texture, projection_coordinates.xy).r;
	const float current_depth = projection_coordinates.z;
	const float bias = shadow_bias;

	float shadow = 1.0;
	if (use_pdf) {
		vec2 texel_size = 1.0 / textureSize(depth_texture, 0);
		const int pdf_size = 1;
		const float pdf_size_squared = 9.0F;
		for (int x = -pdf_size; x <= pdf_size; ++x) {
			for (int y = -pdf_size; y <= pdf_size; ++y) {
				const vec2 uv = projection_coordinates.xy + vec2(x, y) * texel_size;
				const float pcf_depth = texture(depth_texture, uv).r;
				shadow += pcf_depth - bias > pcf_depth ? 1.0 : SHADOWED_AMBIENT_LIGHT;
			}
		}
		shadow /= pdf_size_squared;
	} else {
		shadow = current_depth - bias >= shadow_map_closest_depth ? 1.0 : SHADOWED_AMBIENT_LIGHT;
	}

	if (projection_coordinates.z >= 1.0 || projection_coordinates.z <= -1.0) {
		shadow = 1.0;
	}

	return shadow;
}

vec3 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 view_direction, float shadow, uint spec_pow)
{
	const vec3 light_direction = normalize(-light.direction);
	// diffuse shading
	const float diff = max(dot(normal, light_direction), 0.0);
	// specular shading
	const vec3 reflection_direction = reflect(light_direction, normal);
	const float spec = pow(max(dot(view_direction, reflection_direction), 0.0), spec_pow);
	// combine results
	vec3 ambient = light.ambient;
	vec3 diffuse = light.diffuse * diff;
	vec3 specular = light.specular * spec;
	const float inverse_shadow_factor = 1.0 - shadow;
	return (ambient + inverse_shadow_factor * (diffuse + specular));
	// return vec3(inverse_shadow_factor);
}

vec3 calculate_point_light(vec4 position, vec4 input_factors, vec4 input_ambient, vec4 input_diffuse, vec4 input_specular, vec3 normal, vec3 fragPos,
	float shadow, vec3 view_direction)
{
	const vec3 light_to_frag_vector = vec3(position) - fragPos;
	const vec3 light_direction = normalize(light_to_frag_vector);
	// diffuse shading
	const float diff = max(dot(normal, vec3(light_direction)), 0.0);
	// specular shading
	const vec3 reflection_direction = reflect(-light_direction, normal);
	const float spec = pow(max(dot(view_direction, reflection_direction), 0.0), 32);
	// attenuation
	const float distance_to_light = length(light_to_frag_vector);

	const float factors = dot(vec3(1, input_factors.yz), vec3(1, distance_to_light, distance_to_light * distance_to_light));
	const float attenuation = 1.0F / factors;

	// combine results
	vec3 ambient = vec3(input_ambient);
	vec3 diffuse = vec3(input_diffuse) * diff;
	vec3 specular = vec3(input_specular) * spec;
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	const float inverse_shadow_factor = 1.0 - shadow;
	return (ambient + inverse_shadow_factor * (diffuse + specular));
	// return vec3(inverse_shadow_factor);
}

vec4 gamma_correct(vec4 colour)
{
	const float gamma = 2.2;
	return vec4(pow(colour.rgb, vec3(1.0 / gamma)), 1.0f);
}
