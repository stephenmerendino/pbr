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
cbuffer matrix_buffer_cb : register(b0)
{
	float4x4 MODEL;
	float4x4 VIEW;
	float4x4 PROJECTION;
};



// -------------------------------------------------------------
// Vertex Format(s)
// -------------------------------------------------------------
struct vertex_in_t
{
	float3 position : POSITION;
	float4 tint : TINT;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
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



// -------------------------------------------------------------
// Helper Functions
// -------------------------------------------------------------



// -------------------------------------------------------------
// Vertex Shader
// -------------------------------------------------------------
vertex_to_fragment_t VertexFunction(vertex_in_t vertex)
{
	vertex_to_fragment_t out_data = (vertex_to_fragment_t)0;

	float4 worldSpaceVertex = mul(float4(vertex.position, 1.0f), MODEL);
	float4 cameraSpaceVertex = mul(float4(worldSpaceVertex), VIEW);
	float4 clipSpaceVertex = mul(float4(cameraSpaceVertex), PROJECTION);

	out_data.position = clipSpaceVertex;
	out_data.worldPosition = worldSpaceVertex.xyz;
	out_data.tint = vertex.tint;
	out_data.texCoord = vertex.texCoord;

	out_data.normal = mul(float4(vertex.normal, 0.0f), MODEL).xyz;
    out_data.tangent = mul(float4(vertex.tangent, 0.0f), MODEL).xyz;
    out_data.bitangent = mul(float4(vertex.bitangent, 0.0f), MODEL).xyz;
    
	return out_data;
}