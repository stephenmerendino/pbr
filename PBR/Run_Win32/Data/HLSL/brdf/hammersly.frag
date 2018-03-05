#include "Data/HLSL/Util/util.h"
#include "Data/HLSL/Util/random.h"
#include "Data/HLSL/Util/cb_common.h"
#include "Data/HLSL/Util/brdf.h"

// -------------------------------------------------------------
// Textures
// -------------------------------------------------------------


// -------------------------------------------------------------
// Samplers
// -------------------------------------------------------------


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
    /* Testing out hammersly points */
	float cos_theta = data.texCoord.x;
	float roughness = 1.0f - data.texCoord.y;

    uint2 wh = uint2(128, 128); // grid size to show
    uint x = (uint)(data.texCoord.x * (float)wh.x);
    uint y = (uint)(data.texCoord.y * (float)wh.y);

    uint i = x + (y * wh.x);
    uint total_samples = wh.x * wh.y;

	float2 sample = get_2d_sample(i, total_samples);

    float out_x = sample.x;
    float out_y = sample.y;

	return float4(out_x, out_y, 0, 1);
}