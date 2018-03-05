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

float4 integrate_diffuse_cube(float3 world_normal)
{
    float3 accum = 0.0f;
    uint num_samples = 2048;
    for(uint i = 0; i < num_samples; i++){
		float2 random_sample = get_2d_sample(i, num_samples);

        float3 light;
        float n_dot_l;
        float pdf;

        importance_sample_cos_dir(random_sample, world_normal, light, n_dot_l, pdf);
        if(n_dot_l > 0.0f){
            accum += t_environment.Sample(s_linear, light).rgb * n_dot_l;
        }
    }

    return float4(accum / (float) num_samples, 1.0f);
}


// -------------------------------------------------------------
// Fragment Shader
// -------------------------------------------------------------
float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
    float3 world_normal = normalize(data.worldPosition.xyz);
    return integrate_diffuse_cube(world_normal);
}