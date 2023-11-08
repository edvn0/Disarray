struct DirectionalLight {
    vec3 direction;
    vec4 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define SHADOWED_AMBIENT_LIGHT 0.01

vec3 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 view_direction, float shadow, uint spec_pow);

vec3 calculate_point_light(
vec4 position, vec4 factors, vec4 ambient, vec4 diffuse, vec4 specular, vec3 normal, vec3 fragPos, float shadow, vec3 view_direction);
vec3 calculate_spot_light(
vec4 position, vec4 factors, vec4 ambient, vec4 diffuse, vec4 specular, vec3 normal, vec3 fragPos, float shadow, vec3 view_direction);
float shadow_calculation(vec4 view_projection, sampler2D depth_texture, bool use_pdf, float bias);
float shadow_calculation(vec4 view_projection, sampler2D depth_texture) { return shadow_calculation(view_projection, depth_texture, false, 0.005); }
float shadow_calculation(vec4 view_projection, sampler2D depth_texture, bool use_pdf)
{
    return shadow_calculation(view_projection, depth_texture, false, 0.005);
}
vec4 gamma_correct(vec4 colour);

float shadow_projection(vec4 shadow_coordinates, vec2 off, sampler2D depth_texture, float bias)
{
    const float ambient = SHADOWED_AMBIENT_LIGHT;
    vec2 uvs = shadow_coordinates.xy + off;
    if (shadow_coordinates.z > 1) return ambient;

    return shadow_coordinates.z - bias > texture(depth_texture, uvs).r ? 1.0 : ambient;
}

float pcf(vec4 sc, sampler2D depth_texture, float scale, float shadow_bias);
float pcf(vec4 sc, sampler2D depth_texture, float shadow_bias) { return pcf(sc, depth_texture, 1.5F, shadow_bias); }
float pcf(vec4 sc, sampler2D depth_texture) { return pcf(sc, depth_texture, 1.5F, 0.05F); }

float shadow_calculation(vec4 view_projection, sampler2D depth_texture, bool use_pdf, float shadow_bias)
{
    vec4 projection_coordinates = view_projection / view_projection.w;
    return use_pdf ? pcf(projection_coordinates, depth_texture, shadow_bias)
    : shadow_projection(projection_coordinates, vec2(0.0, 0.0), depth_texture, shadow_bias);
}

vec3 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 view_direction, float shadow, uint spec_pow)
{
    const vec3 light_direction = normalize(-light.direction);
    // diffuse shading
    const float diff = max(dot(normal, light_direction), 0.0);
    // specular shading
    const vec3 reflection_direction = reflect(-light_direction, normal);
    const float spec = pow(max(dot(view_direction, reflection_direction), 0.0), spec_pow);
    // combine results
    vec3 ambient = light.ambient.xyz * light.ambient.a;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    const float inverse_shadow_factor = 1.0F - shadow;
    return (ambient + inverse_shadow_factor * (diffuse + specular));
}

vec3 calculate_point_light(vec4 position, vec4 input_factors, vec4 input_ambient, vec4 input_diffuse, vec4 input_specular, vec3 normal, vec3 fragPos,
float shadow, vec3 view_direction)
{
    const vec3 light_to_frag_vector = vec3(position) - fragPos;
    const vec3 light_direction = normalize(light_to_frag_vector);
    // diffuse shading
    const float diff = max(dot(normal, light_direction), 0.0);
    // specular shading
    const vec3 reflection_direction = reflect(-light_direction, normal);
    const float spec = pow(max(dot(view_direction, reflection_direction), 0.0), 32);
    // attenuation
    const float distance_to_light = length(light_to_frag_vector);

    const float factors = dot(vec3(1, input_factors.yz), vec3(1, distance_to_light, distance_to_light * distance_to_light));
    const float attenuation = 1.0F / factors;

    // combine results
    vec4 ambient = input_ambient;
    vec4 diffuse = input_diffuse * diff;
    vec4 specular = input_specular * spec;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    const float inverse_shadow_factor = 1 - shadow;
    return vec3(ambient + inverse_shadow_factor * (diffuse + specular));
}

vec3 calculate_spot_light(vec4 position, vec4 input_factors, vec4 input_ambient, vec4 input_diffuse, vec4 input_specular, vec3 normal, vec3 fragPos,
float shadow, vec3 view_direction, vec3 spot_light_direction, float spot_light_cutoff)
{
    const vec3 light_to_frag_vector = vec3(position) - fragPos;
    const vec3 light_direction = normalize(light_to_frag_vector);
    const float spot_factors = dot(light_direction, spot_light_direction);

    if (spot_factors > spot_light_cutoff) {
        vec3 colour = calculate_point_light(position, input_factors, input_ambient, input_diffuse, input_specular, normal, fragPos, shadow, view_direction);
        return colour * (1.0 - (1.0 - spot_factors) * 1.0/(1.0 - spot_light_cutoff));
    }
    else {
        return vec3(0, 0, 0);
    }
}

vec4 gamma_correct(vec4 colour)
{
    const float gamma = 2.2;
    return vec4(pow(colour.rgb, vec3(1.0 / gamma)), 1.0f);
}

float pcf(vec4 sc, sampler2D depth_texture, float scale, float shadow_bias)
{
    ivec2 texture_dimensions = textureSize(depth_texture, 0);
    float dx = scale * 1.0 / float(texture_dimensions.x);
    float dy = scale * 1.0 / float(texture_dimensions.y);

    float shadow_factor = 0.0;
    const int count = 9;
    const int range = 1;

    for (int x = -range; x <= range; x++) {
        for (int y = -range; y <= range; y++) {
            shadow_factor += shadow_projection(sc, vec2(dx * x, dy * y), depth_texture, shadow_bias);
        }
    }
    return shadow_factor / count;
}
