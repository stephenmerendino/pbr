#include "Data/HLSL/Util/util.h"
#include "Data/HLSL/Util/cb_common.h"

// -------------------------------------------------------------
// Textures
// -------------------------------------------------------------
Texture2D <float4> t_diff : register(t0);
Texture2D <float4> t_normal : register(t1);
Texture2D <float4> t_spec : register(t2);



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

float3 calc_lambert_shading(float3 normal, 
							float3 light, 
							float3 view,
							float3 diff,
							float3 spec,
							float smoothness,
							float3 light_irradiance)
{
	float n_dot_l = saturate(dot(normal, light));
	float3 surface_irradiance = light_irradiance * n_dot_l;

	float3 h = normalize(light + view);
	float cos_nh = saturate(dot(normal, h));
	float3 spec_factor = saturate(pow(abs(cos_nh), smoothness)) * spec;

	float3 total = (diff + spec_factor) * surface_irradiance;

	return total;
}

float attenuate_distance(float2 cutoff, float distance)
{
	if(distance <= cutoff.x){
		return 1.0f;
	}

	if(distance >= cutoff.y){
		return 0.0f;
	}

	return ((cutoff.y - distance) / (cutoff.y - cutoff.x));
}

float attenuate_direction(float3 light_dir, float3 light_to_surface_dir, float inner_angle, float outer_angle, float exp)
{
	float cos_s = dot(light_dir, light_to_surface_dir);	
	float cos_inner = cos(deg_to_rad(inner_angle));
	float cos_outer = cos(deg_to_rad(outer_angle));

	if(cos_s >= cos_inner){
		return 1.0f;
	}

	if(cos_s <= cos_outer){
		return 0.0f;
	}

	return pow(abs((cos_s - cos_outer) / (cos_inner - cos_outer)), exp);
}



// -------------------------------------------------------------
// Fragment Shader
// -------------------------------------------------------------
float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	// get lambert diffuse term
	float3 diffuse_sample = t_diff.Sample(s_linear, data.texCoord).xyz;
	float3 diff = diffuse_sample / PI;

	// get lambert spec term
	float3 spec = t_spec.Sample(s_linear, data.texCoord).xyz;
	float smoothness = MIDI.KNOB_0 * 8.0f;
	spec *= (smoothness + 8) / (8 * PI);

	// get surface normal
	float3 face_normal = normalize(data.normal);
    float3 tangent = normalize(data.tangent);	
    float3 bitangent = normalize(data.bitangent);

    float3x3 tbn = float3x3(tangent, bitangent, face_normal);

    float3 color_normal = t_normal.Sample(s_linear, data.texCoord).rgb;
    float3 surface_normal = texture_normal_to_surface_normal(color_normal);
	surface_normal = mul(surface_normal, tbn);

	float3 to_view = normalize(EYE_WORLD_POSITION.xyz - data.worldPosition.xyz);

	float3 total_radiance = float3(0.0f, 0.0f, 0.0f);

	// sum up all directional light factors
    for(int dir_idx = 0; dir_idx < 4; ++dir_idx){
		directional_light_t dir_light = DIRECTIONAL_LIGHTS[dir_idx];

		float3 to_light = dir_light.direction.xyz;
		float3 light_irradiance = dir_light.color.xyz * dir_light.color.w;

		total_radiance += calc_lambert_shading(surface_normal,
											   to_light,
											   to_view,
											   diff,
											   spec,
											   smoothness,
		                                       light_irradiance);
    }

	// sum up all point light factors
	for(int pl_idx = 0; pl_idx < 16; ++pl_idx){
		point_light_t pl = POINT_LIGHTS[pl_idx];

		float3 to_light_disp = pl.position.xyz - data.worldPosition;
		float3 to_light_dir = normalize(to_light_disp);

		float light_distance = length(to_light_disp);
		float3 light_irradiance = pl.color.xyz * pl.color.w * attenuate_distance(pl.cutoff, light_distance);

		total_radiance += calc_lambert_shading(surface_normal,
											   to_light_dir,
											   to_view,
											   diff,
											   spec,
											   smoothness,
		                                       light_irradiance);
	}

	// sum up all spot light factors
	for(int sl_idx = 0; sl_idx < 16; ++sl_idx){
		spot_light_t sl = SPOT_LIGHTS[sl_idx];

		float3 to_light_disp = sl.position.xyz - data.worldPosition;
		float3 to_light_dir = normalize(to_light_disp);

		float light_distance = length(to_light_disp);
		float3 light_irradiance = sl.color.xyz * sl.color.w;
		light_irradiance *= attenuate_distance(sl.cutoff, light_distance);
		light_irradiance *= attenuate_direction(sl.direction.xyz, -to_light_dir, sl.inner_angle, sl.outer_angle, sl.exp);

		total_radiance += calc_lambert_shading(surface_normal,
											   to_light_dir,
											   to_view,
											   diff,
											   spec,
											   smoothness,
		                                       light_irradiance);
	}

    return float4(total_radiance, 1.0f);
}