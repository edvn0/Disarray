#include "Buffers.glsl"
#include "MathHelpers.glsl"

#include "DefaultInput.glsl"

layout(location = 0) out flat uint identifier;

void main()
{
    mat4 object_matrix = EntityTransformSSBO.ssbo_objects[gl_InstanceIndex].transform;
    uint object_identifier = EntityIdentifierSSBO.ssbo_objects[gl_InstanceIndex].identifier;

    vec4 frag_pos = object_matrix * vec4(pos, 1.0);
    gl_Position = ubo.view_projection * frag_pos;
    identifier = object_identifier;
}
