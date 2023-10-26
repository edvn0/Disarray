layout(location = 0) in vec3 uvs;

layout(location = 0) out vec4 colour;

layout(set = 2, binding = 4) uniform samplerCube skybox_texture;

void main()
{
    colour = texture(skybox_texture, uvs);
}
