#include "UBO.glsl"

layout(location = 0) in flat uint in_identifier;
layout(location = 0) out uint out_identifier;

void main()
{
	out_identifier = in_identifier;
}
