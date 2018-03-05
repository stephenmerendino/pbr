#include "Data/HLSL/Util/util.h"
#include "Data/HLSL/Util/cb_common.h"

// -------------------------------------------------------------
// Textures
// -------------------------------------------------------------
Texture2D<float4> t_pano : register(t0);


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




// -------------------------------------------------------------
// Fragment Shader
// -------------------------------------------------------------
float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	float3 world_dir = normalize(data.worldPosition.xyz);
	float3 world_xz = normalize(float3(world_dir.x, 0.0f, world_dir.z));

	float u = 0.0f;
	float v = 0.0f;

	float theta_rads = atan2(world_xz.z, world_xz.x);

	if(theta_rads >= 0.0f && theta_rads <= PI){
		u = range_map(theta_rads, 0.0f, PI, 0.0f, 0.5f);
	} else{
		u = range_map(theta_rads, -PI, 0.0f, 0.5f, 1.0f);
	}

	v = acos(world_dir.y) / PI;

	return t_pano.Sample(s_linear, float2(u, v));
}