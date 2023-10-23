#include "PC.glsl"
#include "UBO.glsl"

layout(push_constant) uniform PushConstantBlock { PushConstant pc; }
PC;

layout(location = 0) out uint out_identifier;

void main()
{
	PushConstant pc = PC.pc;
	out_identifier = pc.current_identifier;
}
