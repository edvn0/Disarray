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
float shadow_calculation(vec4 view_projection, sampler2D depth_texture) { return shadow_calculation(view_projection, depth_texture, false, 0.005); }
float shadow_calculation(vec4 view_projection, sampler2D depth_texture, bool use_pdf)
{
    return shadow_calculation(view_projection, depth_texture, false, 0.005);
}
vec4 gamma_correct(vec4 colour);

float shadow_projection(vec4 shadow_coordinates, vec2 off, sampler2D depth_texture)
{
    const float ambient = 0.1f;
    float shadow = 1.0;
    if (shadow_coordinates.z > -1.0 && shadow_coordinates.z < 1.0)
    {
        float dist = texture(depth_texture, shadow_coordinates.st + off).r;
        if (shadow_coordinates.w > 0.0 && dist < shadow_coordinates.z)
        {
            shadow = ambient;
        }
    }
    return shadow;
}

float pcf(vec4 sc, sampler2D depth_texture)
{
    ivec2 texDim = textureSize(depth_texture, 0);
    float scale = 1.5;
    float dx = scale * 1.0 / float(texDim.x);
    float dy = scale * 1.0 / float(texDim.y);

    float shadow_factor = 0.0;
    const int count = 9;
    const int range = 1;

    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            shadow_factor += shadow_projection(sc, vec2(dx*x, dy*y), depth_texture);
        }

    }
    return shadow_factor / count;
}

float shadow_calculation(vec4 view_projection, sampler2D depth_texture, bool use_pdf, float shadow_bias)
{
    vec4 projection_coordinates = view_projection / view_projection.w;
    return use_pdf ? pcf(projection_coordinates, depth_texture) : shadow_projection(projection_coordinates, vec2(0.0, 0.0), depth_texture);
}

vec3 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 view_direction, float shadow, uint spec_pow)
{
    const vec3 light_direction = normalize(light.direction);
    // diffuse shading
    const float diff = max(dot(normal, light_direction), 0.0);
    // specular shading
    const vec3 reflection_direction = reflect(light_direction, normal);
    const float spec = pow(max(dot(view_direction, reflection_direction), 0.0), spec_pow);
    // combine results
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    const float inverse_shadow_factor = 1 - shadow;
    return (ambient + (diffuse + specular));
}

vec3 calculate_point_light(vec4 position, vec4 input_factors, vec4 input_ambient, vec4 input_diffuse, vec4 input_specular, vec3 normal, vec3 fragPos,
float shadow, vec3 view_direction)
{
    const vec3 light_to_frag_vector = vec3(position) - fragPos;
    const vec3 light_direction = normalize(-light_to_frag_vector);
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
    vec4 ambient = input_ambient;
    vec4 diffuse = input_diffuse * diff;
    vec4 specular = input_specular * spec;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    const float inverse_shadow_factor = 1 - shadow;
    return vec3(ambient + (diffuse + specular));
}

vec4 gamma_correct(vec4 colour)
{
    const float gamma = 2.2;
    return vec4(pow(colour.rgb, vec3(1.0 / gamma)), 1.0f);
}
