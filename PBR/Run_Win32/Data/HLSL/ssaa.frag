#include "Data/HLSL/Util/util.h"

Texture2D <float4> tDiffuse : register(t0);

SamplerState sSampler : register(s0);

cbuffer aa_cb : register(b5)
{
	uint ssaa_level;
	float3 _padding;
};

struct vertex_to_fragment_t
{
	float4 position : SV_Position;
	float3 worldPosition : WORLD_POSITION;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

void rotate(inout float2 v, float deg)
{
	float rads = deg_to_rad(deg);
	float len = length(v);
	float angle = atan2(v.y, v.x);

	v.x = len * cos(angle + rads);
	v.y = len * sin(angle + rads);
	//float2 shifted = float2(sample_u, sample_v) + basis_shift;

	//float2 normalized_shifted_uv = normalize(shifted);

	//float rot_deg = 26.6;
	//float rot_rad = deg_to_rad(rot_deg);
	//float2 rotated = 0.0f;

	//rotated.x = cos(rot_rad) * normalized_shifted_uv.x + sin(rot_rad) * normalized_shifted_uv.y;
	//rotated.y = -sin(rot_rad) * normalized_shifted_uv.x + cos(rot_rad) * normalized_shifted_uv.y;

	//float original_length = length(shifted);
	//rotated *= original_length;

	//rotated -= basis_shift;

	//float4 sub_sample = tDiffuse.Sample(sSampler, rotated);
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	// early out if we aren't doing SSAA, just sample and return
	if(ssaa_level == 1){
		return tDiffuse.Sample(sSampler, data.texCoord);
	}

	int half_grid_size = ssaa_level / 2;

	uint2 texture_size = get_texture_size(tDiffuse);

	uint ssaa_w = texture_size.x;
	uint ssaa_h = texture_size.y;

	float u = data.texCoord.x;
	float v = data.texCoord.y;

	float u_texel_offset = (1.0f / (float)ssaa_w);
	float v_texel_offset = (1.0f / (float)ssaa_h);

	float4 accum = 0.0f;

	// Grid Sampling
	/*/

	float2 uv = float2(u, v);
	uv.x -= ((u_texel_offset * half_grid_size) + ((half_grid_size - 1) * u_texel_offset));
	uv.y -= ((v_texel_offset * half_grid_size) + ((half_grid_size - 1) * v_texel_offset));

	for(uint i = 0; i < ssaa_level; i++){
		for(uint j = 0; j < ssaa_level; j++){
			float2 s = uv + float2(i * u_texel_offset, j * v_texel_offset);
			float4 sub_sample = tDiffuse.Sample(sSampler, s);
			accum += sub_sample;
		}
	}

	/*/

	// Rotated Grid Sampling
	float2 basis_shift = float2(-u_texel_offset * half_grid_size, -v_texel_offset * half_grid_size);

	basis_shift -= float2(-u_texel_offset * 0.5f, -v_texel_offset * 0.5f);

	for(uint u_idx = 0; u_idx < ssaa_level; u_idx++){
		for(uint v_idx = 0; v_idx < ssaa_level; v_idx++){

			float u_local_offset = u_texel_offset * (float)u_idx;
			float v_local_offset = v_texel_offset * (float)v_idx;

			float2 local_offset = float2(u_local_offset, v_local_offset);
			local_offset -= basis_shift;
			rotate(local_offset, 26.6f);
			local_offset += basis_shift;

			float sample_u = u + local_offset.x;
			float sample_v = v + local_offset.y;

			float4 sub_sample = tDiffuse.Sample(sSampler, float2(sample_u, sample_v));
			accum += sub_sample;

		}
	}
	/**/

	accum /= (float)(ssaa_level * ssaa_level);
	return accum;
}