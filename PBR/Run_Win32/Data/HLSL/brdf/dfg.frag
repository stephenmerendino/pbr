#include "Data/HLSL/Util/util.h"
#include "Data/HLSL/Util/random.h"
#include "Data/HLSL/Util/cb_common.h"
#include "Data/HLSL/brdf/brdf.h"

// -------------------------------------------------------------
// Textures
// -------------------------------------------------------------
TextureCube t_environment : register(t6);


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
// Helper Functions
// -------------------------------------------------------------

// Pure Smith GGX without the folded in BRDF term
float ggx(float dot, float a2)
{
    float numerator = 2.0f * dot;
    float denominator = dot + sqrt(a2 + (1.0f - a2) * (dot * dot));
    return numerator / denominator;
}

float G_smith(float n_dot_v, float n_dot_l, float non_linear_roughness)
{
    float a = non_linear_roughness * non_linear_roughness;
    float a2 = a * a;

    return ggx(n_dot_v, a2) * ggx(n_dot_l, a2);
}

float3 integrate_brdf_dfg(float n_dot_v, float non_linear_roughness)
{
    float linear_roughness = non_linear_roughness * non_linear_roughness;

    float3 accum = 0.0f;

    float3 view;
    view.x = sqrt(1.0f - n_dot_v * n_dot_v);
    view.y = 0.0f;
    view.z = n_dot_v;

    float3 normal = float3(0.0f, 0.0f, 1.0f);

    //const uint sample_count = (uint) range_map(MIDI.SLIDER_0, 0.0f, 1.0f, 2.f, 1024.f);
    const uint sample_count = 1024;
    for(uint i = 0; i < sample_count; i++){
        float2 Xi = hammersly(i, sample_count);
        float3 half_vec = importance_sample_ggx(Xi, non_linear_roughness, normal);
        float3 light = 2.0f * dot(view, half_vec) * half_vec - view;

        float n_dot_l = saturate(light.z);
        float n_dot_h = saturate(half_vec.z);
        float v_dot_h = saturate(dot(view, half_vec));
        float l_dot_h = saturate(dot(light, half_vec));

        // specular pre-integration
        if(n_dot_l > 0.0f){
            // Frostbite/Epic
            // float G = G_smith(n_dot_v, n_dot_l, non_linear_roughness);
            // float G_Vis = G * l_dot_h / (n_dot_h * n_dot_v);

            // This uses the optimized version given by frostbite but cancels out the brdf terms
            float G = G_calc_smith_ggx(n_dot_v, n_dot_l, non_linear_roughness);
            float G_Vis = G * (4.0f * n_dot_l * v_dot_h / n_dot_h);

            float Fc = pow(1.0f - v_dot_h, 5.0f);
            accum.x += (1.0f - Fc) * G_Vis;
            accum.y += Fc * G_Vis;
        }

        // difuse pre-integration
        Xi = frac(Xi + 0.5f); // why do this??
        float pdf;

        importance_sample_cos_dir(Xi, normal, light, n_dot_l, pdf);
        if(n_dot_l > 0.0f){
            l_dot_h = saturate(dot(light, normalize(view + light)));
            n_dot_v = saturate(dot(normal, view));
            accum.z += calculate_diffuse_energy(l_dot_h, n_dot_v, n_dot_l, non_linear_roughness);
        }
    }

    return accum / (float) sample_count;
}

// -------------------------------------------------------------
// Fragment Shader
// -------------------------------------------------------------
float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
    /* Testing out hammersly points */
    /*/
	float cos_theta = data.texCoord.x;
	float roughness = 1.0f - data.texCoord.y;

    uint2 wh = uint2(128, 128);
    uint x = (uint)(data.texCoord.x * (float)wh.x);
    uint y = (uint)(data.texCoord.y * (float)wh.y);

    uint i = x + (y * wh.x);
    uint total_samples = wh.x * wh.y;

	float2 sample = get_2d_sample(i, total_samples);

    float out_x = sample.x;
    float out_y = sample.y;

	return float4(out_x, out_y, 0, 1);
    /**/

	float n_dot_v = data.texCoord.x;
	float non_linear_roughness = data.texCoord.y;

    float3 dfg = integrate_brdf_dfg(n_dot_v, non_linear_roughness);
    return float4(dfg, 1.0f);
}