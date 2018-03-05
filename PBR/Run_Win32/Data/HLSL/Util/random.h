#include "Data/HLSL/Util/noise.h"

float radical_inverse(uint bits)
{
	/**/
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000

	/*/

	bits = ((bits >> 1) & 0x55555555) | ((bits & 0x55555555) << 1);
	bits = ((bits >> 2) & 0x33333333) | ((bits & 0x33333333) << 2);
	bits = ((bits >> 4) & 0x0F0F0F0F) | ((bits & 0x0F0F0F0F) << 4);
	bits = ((bits >> 8) & 0x00FF00FF) | ((bits & 0x00FF00FF) << 8);
	bits = (bits >> 16) | (bits << 16);
	return ((float)bits / (float)0x10000000);
	/**/
}

float2 hammersly(uint i, uint num_samples)
{
	float2 sample;
	sample.x = (float)i / (float)num_samples;
	sample.y = radical_inverse(i);
	return sample;
}

float2 get_bad_sample(float u, float v)
{
	float2 sample;
	sample.x = Get1dNoiseZeroToOne(u * 4951.0f, v * 1283.0f);
	sample.y = Get1dNoiseZeroToOne(v * 41351.0f, u * 19979.0f);
	return sample;
}

float2 get_2d_sample(uint sample, uint total_samples)
{
	return hammersly(sample, total_samples);
}