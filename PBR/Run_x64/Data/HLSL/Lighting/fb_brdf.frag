#include "Data/HLSL/Util/util.h"
#include "Data/HLSL/Util/cb_common.h"

// -------------------------------------------------------------
// Textures
// -------------------------------------------------------------



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

float3 F_approx_schlick(float3 f0, float3 f90, float u)
{
	return f0 + ((f90 - f0) * pow(abs(1.0f - u), 5.0f));
}

float G_calc_smith_ggx(float n_dot_v, float n_dot_l, float roughness)
{
	float r2 = (roughness * roughness);
	float lambda_ggx_v = n_dot_l * sqrt((-n_dot_v * r2 + n_dot_v) * n_dot_v + r2);
	float lambda_ggx_l = n_dot_v * sqrt((-n_dot_l * r2 + n_dot_l) * n_dot_l + r2);

	return 0.5f / (lambda_ggx_v + lambda_ggx_l);
}

float NDF_calc_ggx(float n_dot_h, float m)
{
	float m2 = (m * m);
	float f = (n_dot_h * m2 - n_dot_h) * n_dot_h + 1;
	return m2 / (f * f);
}

float3 calculate_spec_energy(float3 f0, 
						    float3 f90, 
							float l_dot_h, 
							float n_dot_v, 
							float n_dot_l, 
							float n_dot_h, 
							float roughness)
{
	float3 fresnel = F_approx_schlick(f0, f90, l_dot_h);
	float vis = G_calc_smith_ggx(n_dot_v, n_dot_l, roughness);
	float ndf = NDF_calc_ggx(n_dot_h, roughness);
	return ((fresnel * vis * ndf) / PI);
}

float calculate_diffuse_energy(float l_dot_h, 
							   float n_dot_v, 
							   float n_dot_l, 
							   float linear_roughness)
{
	float energy_bias = lerp(0.0f, 0.5f, linear_roughness);
	float energy_factor = lerp(1.0f, 1.0f / 1.51f, linear_roughness);
	float fd90 = energy_bias + 2.0f * l_dot_h * l_dot_h * linear_roughness;
	float3 f0 = float3(1.0f, 1.0f, 1.0f);
	float light_scatter = F_approx_schlick(f0, fd90, n_dot_l).r;
	float view_scatter = F_approx_schlick(f0, fd90, n_dot_v).r;

	return (light_scatter * view_scatter * energy_factor) / PI;
}

float attenuate_distance(float2 cutoff, float distance)
{
	// make sure distance isn't too small to prevent numerical explosion
    distance = max(distance, 0.00001f);

	float sq_dist = distance * distance;
	float inv_sq_att_radius = 1.0f / (cutoff.y * cutoff.y);
	float factor = sq_dist * inv_sq_att_radius;
	float smooth_factor = saturate(1.0f - factor * factor);
	smooth_factor *= smooth_factor;

    return (1.0f / (distance * distance)) * smooth_factor;
}

float3 get_albedo()
{
	float3 albedo = MAT_PARAMS.albedo.xyz;
    return albedo;
}

// -------------------------------------------------------------
// Fragment Shader
// -------------------------------------------------------------
float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	if(mat_uses_albedo_texture(MAT_PARAMS.flags)){
		return float4(0.0f, 0.0f, 1.0f, 1.0f);
	}

	// using the first point light for testing purposes
    point_light_t pl = POINT_LIGHTS[0];

	//#TODO: check if this needs to be a texture look up or a constant value
	float smoothness = MAT_PARAMS.smoothness;
    float roughness = saturate(1.0f - smoothness);
	float linear_roughness = roughness * roughness;

    float3 albedo = get_albedo();

    float3 f0 = 0.0f;
    if(mat_is_metal(MAT_PARAMS.flags)){
        f0 = albedo;
    }else{
    	float3 reflectance = MAT_PARAMS.reflectance.xyz;
        f0 = (reflectance * reflectance) * 0.16f;
    }

	float3 f90 = float3(1.00f, 1.00f, 1.00f);

	// -------------------------------------------------------------------------
	// calculate needed values
	float3 view = normalize(EYE_WORLD_POSITION.xyz - data.worldPosition.xyz);
	float3 light = normalize(pl.position.xyz - data.worldPosition.xyz);
	float3 normal = normalize(data.normal);

	float n_dot_v = abs(dot(normal, view)) + 1e-2f; // avoid artifact
	float3 h = normalize(view + light);
	float l_dot_h = saturate(dot(light, h));
	float n_dot_h = saturate(dot(normal, h));
	float n_dot_l = saturate(dot(normal, light));

	// -------------------------------------------------------------------------
	// calculate diff and spec values
	float3 spec_energy = calculate_spec_energy(f0, f90, l_dot_h, n_dot_v, n_dot_l, n_dot_h, roughness);
	float3 diff_energy = calculate_diffuse_energy(l_dot_h, n_dot_v, n_dot_l, linear_roughness);

	// -------------------------------------------------------------------------
	// calculate lighting
	float3 to_light_disp = pl.position.xyz - data.worldPosition.xyz;
	float light_distance = length(to_light_disp);
    float3 light_irradiance = pl.color.xyz * pl.color.w * attenuate_distance(pl.cutoff, light_distance) * (1.0f / 4.0f * PI);

	// -------------------------------------------------------------------------
	// combine terms and figure out final fragment value
    float3 final = 0.0f;
    if (mat_is_metal(MAT_PARAMS.flags)){
        final = spec_energy * light_irradiance * n_dot_l; // metals absorb all diffuse light and only have a specular response
    }
    else{
    	final = ( ( (albedo * diff_energy) + spec_energy ) * light_irradiance * n_dot_l);
    }

    final = pow(abs(final), 1.0f / 2.2f);

	return float4(final, 1.0f);
}