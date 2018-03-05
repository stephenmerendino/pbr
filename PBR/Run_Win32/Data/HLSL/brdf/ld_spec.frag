#include "Data/HLSL/Util/util.h"
#include "Data/HLSL/Util/random.h"
#include "Data/HLSL/Util/cb_common.h"
#include "Data/HLSL/brdf/brdf.h"

// -------------------------------------------------------------
// Textures
// -------------------------------------------------------------
TextureCube t_environment : register(t0);


// -------------------------------------------------------------
// Samplers
// -------------------------------------------------------------
SamplerState s_linear : register(s0);
SamplerState s_point : register(s1);



// -------------------------------------------------------------
// Data Structures 
// -------------------------------------------------------------


// -------------------------------------------------------------
// Constant Buffers
// -------------------------------------------------------------
cbuffer mip_info_buffer_cb : register(b9)
{
	uint CURRENT_MIP_LEVEL;
	uint TOTAL_MIP_LEVELS;
    uint ZERO_MIP_WIDTH;
    float _padding;
};


// -------------------------------------------------------------
// Vertex Format(s)
// -------------------------------------------------------------
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


// -------------------------------------------------------------
// IBL
// -------------------------------------------------------------

float3 integrate_spec_ld(float3 normal, float linear_roughness)
{
    float3 view = normal;
    float3 accum = 0.0f;
    float weight = 0.0f;

    const uint sample_count = 1024;
    for(uint i = 0; i < sample_count; ++i){
        float2 Xi = get_2d_sample(i, sample_count);
        float3 half_vec = importance_sample_ggx(Xi, linear_roughness, normal);

		float3 light = normalize(2.0f * dot(view, half_vec) * half_vec - view);
        float n_dot_l = dot(normal, light);
        if(n_dot_l > 0.0f){
            float n_dot_h = saturate(dot(normal, half_vec));
            float l_dot_h = saturate(dot(light, half_vec));

            float pdf = (D_calc_ggx(n_dot_h, linear_roughness) / PI) * n_dot_h / (4 * l_dot_h);
            //float pdf = D_calc_ggx(n_dot_h, linear_roughness) * PI / 4.0f;

            float omega_s = 1.0f / (sample_count * pdf);
            float omega_p = 4.0f * PI / (6.0 * (float)ZERO_MIP_WIDTH * (float)ZERO_MIP_WIDTH);
            float mip_level = clamp(0.5f * log2(omega_s / omega_p), 0, TOTAL_MIP_LEVELS);
            float4 Li = t_environment.SampleLevel(s_linear, light, mip_level);

            accum += Li.rgb * n_dot_l;
            weight += n_dot_l;
        }
    }

    return accum * (1.0f / weight);
}


// -------------------------------------------------------------
// Fragment Shader
// -------------------------------------------------------------
float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
    float3 world_normal = normalize(data.worldPosition.xyz);

    float linear_roughness = (float) CURRENT_MIP_LEVEL / (float) TOTAL_MIP_LEVELS;
    linear_roughness *= linear_roughness;

    // first mip level is the exact same as the input cubemap
    // this allows us to do mirror-like specular reflections
    if(CURRENT_MIP_LEVEL == 0){
        return t_environment.SampleLevel(s_linear, world_normal, 0.0f);
    }

    return float4(integrate_spec_ld(world_normal, linear_roughness), 1.0f);
}