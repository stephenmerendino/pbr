Texture2D tImage : register(t0);
SamplerState sSampler : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    float4x4 MODEL;
    float4x4 VIEW;
    float4x4 PROJECTION;
};

cbuffer TimeBuffer : register(b1)
{
    float GAME_TIME;
    float SYSTEM_TIME;
    float GAME_FRAME_TIME;
    float SYSTEM_FRAME_TIME;
};

cbuffer PointLightBuffer : register(b6)
{
    float4 light_pos;
    float light_far_plane;
    float3 _padding;
}

struct vertex_in_t
{
    float3 position : POSITION;
    float4 tint : TINT;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
};

struct vertex_to_fragment_t
{
    float4 position : SV_Position;
    float3 worldPosition : WORLD_POSITION;
    float4 tint : TINT;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
};

vertex_to_fragment_t VertexFunction(vertex_in_t vertex)
{
    vertex_to_fragment_t out_data = (vertex_to_fragment_t) 0;

    float4 worldSpaceVertex = mul(float4(vertex.position, 1.0f), MODEL);
    float4 cameraSpaceVertex = mul(float4(worldSpaceVertex), VIEW);
    float4 clipSpaceVertex = mul(float4(cameraSpaceVertex), PROJECTION);

    out_data.position = clipSpaceVertex;
    out_data.worldPosition = worldSpaceVertex.xyz;
    out_data.tint = vertex.tint;
    out_data.texCoord = vertex.texCoord;

    return out_data;
}

struct frag_out_t
{
    float4 color : SV_Target0;
    float depth : SV_Depth;
};

frag_out_t FragmentFunction(vertex_to_fragment_t data)
{
    float3 displacement = data.worldPosition - light_pos.xyz;
    float depth = length(displacement) / light_far_plane;

    frag_out_t out_data;
    out_data.color = float4(data.position.z, 0.0f, 0.0f, 1.0f);
    out_data.depth = depth;

    return out_data;
}
