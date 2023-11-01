/**
N.B. Stolen from https://github.com/Zielon/PBRVulkan/blob/master/PBRVulkan/RayTracer/src/Assets/Shaders/Common/Random.glsl
*/
uint tea(uint val0, uint val1)
{
	uint v0 = val0;
	uint v1 = val1;
	uint s0 = 0;

	[[unroll]] for (uint n = 0; n < 16; n++) {
		s0 += 0x9e3779b9;
		v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
		v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
	}

	return v0;
}

uint lcg(inout uint prev)
{
	uint LCG_A = 1664525u;
	uint LCG_C = 1013904223u;
	prev = (LCG_A * prev + LCG_C);
	return prev & 0x00FFFFFF;
}

float next_float(inout uint prev) { return (float(lcg(prev)) / float(0x01000000)); }
uint next_uint(inout uint prev, uint count)
{
	uint normalised = lcg(prev);
	return normalised % (count - 1);
}
