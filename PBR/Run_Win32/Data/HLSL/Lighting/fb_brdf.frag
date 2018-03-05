#include "Data/HLSL/Util/util.h"
#include "Data/HLSL/Util/random.h"
#include "Data/HLSL/Util/cb_common.h"
#include "Data/HLSL/brdf/brdf.h"

// -------------------------------------------------------------
// Textures
// -------------------------------------------------------------
Texture2D<float4> t_albedo : register(t0);
Texture2D<float4> t_smoothness : register(t1);
Texture2D<float4> t_normal : register(t2);
Texture2D<float4> t_spec : register(t3);
Texture2D<float4> t_metalness : register(t4);
Texture2D<float4> t_ao : register(t5);

TextureCube t_diffuse_ld : register(t7);
TextureCube t_spec_ld : register(t8);
Texture2D<float4> t_dfg : register(t9);

//Texture2D<float4> t_dl0_depth : register(t10);
Texture2D<float4> t_sl0_depth : register(t10);
//TextureCube t_pl0_depth : register(t10);

// -------------------------------------------------------------
// Samplers
// -------------------------------------------------------------
SamplerState s_linear : register(s0);
SamplerState s_point : register(s1);
SamplerComparisonState s_comparison : register(s2);



// -------------------------------------------------------------
// Data Structures 
// -------------------------------------------------------------


// -------------------------------------------------------------
// Constant Buffers
// -------------------------------------------------------------
cbuffer ibl_data_t : register(b5)
{
	uint TOTAL_MIP_LEVELS;
    uint DFG_TEXTURE_SIZE;
    float2 _padding;
}


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
// Param functions
// -------------------------------------------------------------
float3 srgb_to_rgb(float3 srgb)
{
    const float3x3 transform = {0.4124, 0.2126, 0.0193, 
                                0.3576, 0.7152, 0.1192, 
                                0.1805, 0.0722, 0.9505};

    return mul(srgb, transform);
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

float attenuate_direction(float3 light_dir, float3 light_to_surface_dir, float inner_angle, float outer_angle, float exp)
{
    float cos_s = dot(light_dir, light_to_surface_dir);
    float cos_inner = cos(deg_to_rad(inner_angle));
    float cos_outer = cos(deg_to_rad(outer_angle));

    if (cos_s >= cos_inner)
    {
        return 1.0f;
    }

    if (cos_s <= cos_outer)
    {
        return 0.0f;
    }

    return pow(abs((cos_s - cos_outer) / (cos_inner - cos_outer)), exp);
}

float3 get_albedo(vertex_to_fragment_t data)
{
	if(mat_uses_albedo_texture(MAT_PARAMS.flags)){
		return t_albedo.Sample(s_linear, data.texCoord).xyz;
	} else{
		return MAT_PARAMS.albedo.xyz;
	}
}

float3 get_normal(vertex_to_fragment_t data)
{
	float3 normal = normalize(data.normal);

	if(mat_uses_normal_texture(MAT_PARAMS.flags)){
		float3 tangent = normalize(data.tangent);
		float3 bitangent = normalize(data.bitangent);

		float3x3 tbn = float3x3(tangent, bitangent, normal);

		float3 texture_normal = t_normal.Sample(s_linear, data.texCoord).xyz;
		texture_normal *= float3(2.0f, 2.0f, 2.0f);
		texture_normal -= float3(1.0f, 1.0f, 1.0f);

		texture_normal.y *= -1.0f;

		float3 pixel_normal = mul(texture_normal, tbn);
		normal = pixel_normal;
	}

	return normal;
}

float get_roughness(vertex_to_fragment_t data)
{
	float roughness = 0.5f;

	if(mat_uses_smoothness_texture(MAT_PARAMS.flags)){
		float texture_smoothness = t_smoothness.Sample(s_linear, data.texCoord).x;
		roughness = 1.0f - texture_smoothness;
	} else{
		float smoothness = MAT_PARAMS.smoothness;
	    roughness = saturate(1.0f - smoothness);
	}

    roughness = range_map(roughness, 0.0f, 1.0f, 0.015f, 0.999f); // keep it in a range that won't break the equations

	return roughness;
}

float get_metalness(vertex_to_fragment_t data)
{
	float metalness = 0.0f;

	if(mat_uses_metalness_texture(MAT_PARAMS.flags)){
		float sampled_metalness = t_metalness.Sample(s_linear, data.texCoord).x;
		metalness = sampled_metalness;
	} else{
		if(mat_is_metal(MAT_PARAMS.flags)){
			metalness = 1.0f;
		} else{
			metalness = 0.0f;
		}
	}

	return metalness;
}

float3 get_reflectivity(vertex_to_fragment_t data, inout float3 albedo)
{
	float3 f0 = 0.0f;

	if(mat_uses_metalness_texture(MAT_PARAMS.flags)){
		float metalness = get_metalness(data);
		if(metalness > 0.0f){
			f0 = float3(albedo.x, albedo.y, albedo.z);
            //albedo *= saturate((1.0f - metalness));
        }else{
	    	float3 reflectance = MAT_PARAMS.reflectance.xyz;
	        f0 = (reflectance * reflectance) * 0.16f;
        }
	} else{
	    if(mat_is_metal(MAT_PARAMS.flags)){
			f0 = float3(albedo.x, albedo.y, albedo.z);
			albedo = 0.0f;
	    }else{
	    	float3 reflectance = MAT_PARAMS.reflectance.xyz;
	        f0 = (reflectance * reflectance) * 0.16f;
	    }
	}

	return f0;
}

float get_ao(vertex_to_fragment_t data)
{
	if(mat_uses_ao_texture(MAT_PARAMS.flags)){
		return t_ao.Sample(s_linear, data.texCoord).x;
	} else{
		return 1.0f;
	}
}


// -------------------------------------------------------------
// IBL
// -------------------------------------------------------------

//float3 approximate_ibl_spec(float3 normal, float3 view, float roughness, float3 f0, uint sample_count)
//{
//	if(!ENVIRONMENT_MAP_ENABLED){
//		return float3(0.0f, 0.0f, 0.0f);
//	}
//
//	float3 up = saturate(normal.y) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
//	float3 local_right = normalize(cross(up, normal));
//	float3 local_forward = cross(normal, local_right);
//
//	float3 accumulated_light = 0.0f;
//	for(uint i = 0; i < sample_count; i++){
//		// get random sample to build local space direction
//		float2 hammersly = get_2d_sample(i, sample_count);
//        float3 half_vec = importance_sample_ggx(hammersly, roughness, normal);
//
//		float3 light = normalize(2.0f * dot(view, half_vec) * half_vec - view);
//
//		float l_dot_h = abs( dot(light, half_vec) ) + 1e-2f;
//		float n_dot_h = abs( dot(normal, half_vec) ) + 1e-2f;
//		float n_dot_v = abs( dot(normal, view) ) + 1e-2f;
//		float n_dot_l = abs( dot(normal, light) ) + 1e-2f;
//
//		float d = D_calc_ggx(n_dot_h, roughness);
//		float pdf_h = d * n_dot_h;
//		float pdf = pdf_h / (4.0f * l_dot_h);
//
//		if(n_dot_l > 0.0f && pdf > 0.0f){
//			float3 f = F_approx_schlick(f0, 1.0f, l_dot_h);
//
//			float g = G_calc_smith_ggx(n_dot_v, n_dot_l, roughness);
//
//			float3 weight = f * g * d / (4.0f * n_dot_v);
//
//			accumulated_light += (t_environment.Sample(s_linear, light).rgb / PI) * weight / pdf;
//		}
//	}
//
//	return accumulated_light / (float)sample_count;
//}

//float3 approximate_ibl_diffuse(float3 normal, float3 view, float roughness, uint sample_count)
//{
//	float3 accumulated_light = 0.0f;
//
//	for(uint i = 0; i < sample_count; ++i){
//		float2 random_sample = get_2d_sample(i, sample_count);
//
//		float3 light;
//		float n_dot_l;
//		float pdf;
//
//		importance_sample_cos_dir(random_sample, normal, light, n_dot_l, pdf);
//
//		if(n_dot_l > 0.0f){
//			float f = 1.0f;
//
//			float cos_d = sqrt((dot(view, light) + 1.0f) * 0.5f);
//			float n_dot_v = saturate(dot(normal, view));
//
//			float3 f0 = 1.0f;
//			float fd90 = 0.5f + 2.0f * cos_d * cos_d * sqrt(roughness);
//
//			float light_scatter = F_approx_schlick(f0, fd90, n_dot_l).r;
//			float view_scatter = F_approx_schlick(f0, fd90, n_dot_v).r;
//
//			f = light_scatter + view_scatter;
//
//			accumulated_light += t_environment.Sample(s_linear, light).rgb * f;
//		}
//	}
//
//	return accumulated_light / (float)sample_count;
//}

float3 get_spec_dominant_dir(float3 normal, float3 reflected, float roughness)
{
    float smoothness = saturate(1.0f - roughness);
    float lerp_factor = smoothness * (sqrt(smoothness) + roughness);
    return lerp(normal, reflected, lerp_factor);
}

float3 calculate_spec_ibl(float3 world_normal, float3 view, float n_dot_v, float roughness, float3 f0)
{
    float linear_roughness = roughness * roughness;

	float3 reflected = normalize(2.0f * dot(view, world_normal) * world_normal - view);
    float3 dominant_spec_dir = get_spec_dominant_dir(world_normal, reflected, linear_roughness);

    n_dot_v = max(n_dot_v, 0.5f / (float)DFG_TEXTURE_SIZE);
    float mip_level = roughness * TOTAL_MIP_LEVELS;

    float3 ld = t_spec_ld.SampleLevel(s_linear, dominant_spec_dir, mip_level).xyz;

    float2 dfg = t_dfg.Sample(s_linear, float2(n_dot_v, roughness)).xy;

    return ld * (f0 * dfg.x + dfg.y);
}

float3 get_diff_dominant_dir(float3 normal, float3 view, float n_dot_v, float roughness)
{
    float a = 1.02341f * roughness - 1.51174f;
    float b = -0.511705f * roughness + 0.755868f;
    float lerp_factor = saturate((n_dot_v * a + b) * roughness);
    return lerp(normal, view, lerp_factor);
}

float3 calculate_diffuse_ibl(float3 world_normal, float3 view, float n_dot_v, float roughness)
{
    float3 diff_dominant_dir = get_diff_dominant_dir(world_normal, view, n_dot_v, roughness);
    float3 diffuse_sample = t_diffuse_ld.Sample(s_linear, diff_dominant_dir).rgb;
    float dfg = t_dfg.Sample(s_linear, float2(n_dot_v, roughness)).z;
    return diffuse_sample * dfg;
}


// -------------------------------------------------------------
// Shadows
// -------------------------------------------------------------


float compute_spotlight_shadow_factor(spot_light_t sl, float3 frag_world_pos)
{
    if(sl.is_shadow_casting == SHADOW_DISABLED){
        return 1.0f;
    }

    // transform world pos to camera view/proj
    float4 pos_in_camera_space = mul(float4(frag_world_pos, 1.0f), sl.view_proj);
    pos_in_camera_space /= pos_in_camera_space.w;

    float scene_depth = pos_in_camera_space.z;

    // map [-1,1] to [0, 1] uv
    float3 uvw = (pos_in_camera_space.xyz + 1.0f) * 0.5f;
    uvw.y = 1.0f - uvw.y;

    if(uvw.x < 0.0f || uvw.x > 1.0f || uvw.y < 0.0f || uvw.y > 1.0f){
        return 1.0f;
    }

    //float cmp_depth = t_sl0_depth.SampleCmp(s_comparison, uvw.xy, scene_depth - 0.001f).r;
    //return cmp_depth;

    uint2 tex_dim = get_texture_size(t_sl0_depth);
    float2 texel_size = 1.0f / (float2)tex_dim;

    float shadow = 0.0f;

    // poisson sampling
    float2 poisson_values[] =
    {
        float2(-0.94201624, -0.39906216),
        float2(0.94558609, -0.76890725),
        float2(-0.094184101, -0.92938870),
        float2(0.34495938, 0.29387760)
    };

    const uint NUM_SAMPLES = 4;
    float sample_scale = MIDI.SLIDER_0 * 1000.0f;
    [unroll]
    for(uint i = 0; i < NUM_SAMPLES; i++){
        shadow += t_sl0_depth.SampleCmp(s_comparison, uvw.xy + (poisson_values[i] / sample_scale), scene_depth - 0.001f).r;
    }

    return shadow / (float)NUM_SAMPLES;

    // 9xPCF
    //for (int u = -1; u <= 1; ++u){
    //    for (int v = -1; v <= 1; ++v){
    //        float2 sample_uv = uvw.xy + float2(u, v) * texel_size;

    //        // sample depth texture
    //        shadow += t_sl0_depth.SampleCmp(s_comparison, sample_uv, scene_depth - 0.001f).r;
    //    }
    //}

    //return (shadow / 9.0f);
}


// -------------------------------------------------------------
// Fragment Shader
// -------------------------------------------------------------
float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	// -------------------------------------------------------------------------
	// calculate needed values
	float non_linear_roughness = get_roughness(data);
    float3 albedo = get_albedo(data);
    //float3 f0 = get_reflectivity(data, albedo);
	float3 normal = get_normal(data);
	float ao = get_ao(data);
	float3 view = normalize(EYE_WORLD_POSITION.xyz - data.worldPosition.xyz);
	float n_dot_v = abs(dot(normal, view)) + 1e-2f; // avoid artifact
    float metalness = get_metalness(data);

    const float3 f0_spec = albedo;
    const float3 f0_diffuse = float3(0.08f, 0.08f, 0.08f);

    if(MIDI.BUTTON_0){
        return float4(albedo, 1.0f);
    }

    if(MIDI.BUTTON_1){
        return float4(normal, 1.0f);
    }

    if(MIDI.BUTTON_2){
        return float4(f0_spec, 1.0f);
    }

    if(MIDI.BUTTON_3){
        return float4(non_linear_roughness, non_linear_roughness, non_linear_roughness, 1.0f);
    }

    if(MIDI.BUTTON_4){
        return float4(metalness, metalness, metalness, 1.0f);
    }


	// -------------------------------------------------------------------------
	// IBL
	float3 spec_ibl = 0.0f;
	float3 diff_ibl = 0.0f;

	if(ENVIRONMENT_MAP_ENABLED){
        //const uint sample_count = range_map(MIDI.SLIDER_0, 0.0f, 1.0f, 2.f, 1024.f);
		//spec_ibl = approximate_ibl_spec(normal, view, linear_roughness, f0, sample_count);
		//diff_ibl = approximate_ibl_diffuse(normal, view, linear_roughness, sample_count);

        float3 spec_ibl_nm = calculate_spec_ibl(normal, view, n_dot_v, non_linear_roughness, f0_diffuse);
        float3 spec_ibl_m = calculate_spec_ibl(normal, view, n_dot_v, non_linear_roughness, f0_spec);
        spec_ibl = lerp(spec_ibl_nm, spec_ibl_m, metalness);

        diff_ibl = calculate_diffuse_ibl(normal, view, n_dot_v, non_linear_roughness) * (1.0f - metalness);
	}

	// -------------------------------------------------------------------------
	// calculate diff and spec values

    float3 final_diff_energy = 0.0f;
    float3 final_spec_energy = 0.0f;

    // for each point light calc energy
    for(uint i = 0; i < MAX_POINT_LIGHTS; i++){
        point_light_t pl = POINT_LIGHTS[i];

        if(pl.color.w <= 0.001f){
            continue;
        }

        // Spot Light Shadow Mapping with Bias
        //if(i == 0){
        //    float3 world_displacement = data.worldPosition - pl.position.xyz;
        //    float3 light_to_surface_dir = normalize(world_displacement);

        //    float3 light_depth = t_pl0_depth.SampleLevel(s_point, light_to_surface_dir, 0).xyz;

        //    float light_map_depth = light_depth.x * pl.cutoff.y;
        //    float scene_depth = length(world_displacement);

        //    // don't contribute if depth in texture is less than depth in scene
        //    float bias = 0.001f;
        //    if(light_map_depth < scene_depth - bias)
        //    {
        //        continue;
        //    }
        //}

        float3 light = normalize(pl.position.xyz - data.worldPosition.xyz);
    	float3 h = normalize(view + light);
    	float l_dot_h = saturate(dot(light, h));
    	float n_dot_h = saturate(dot(normal, h));
    	float n_dot_l = saturate(dot(normal, light));

    	float3 spec_energy_nm = calculate_spec_energy(f0_diffuse, l_dot_h, n_dot_v, n_dot_l, n_dot_h, non_linear_roughness);
    	float3 spec_energy_m = calculate_spec_energy(f0_spec, l_dot_h, n_dot_v, n_dot_l, n_dot_h, non_linear_roughness);
        float3 spec_energy = lerp(spec_energy_nm, spec_energy_m, metalness);

    	float3 diff_energy = calculate_diffuse_energy(l_dot_h, n_dot_v, n_dot_l, non_linear_roughness);

        diff_energy *= ao;
        spec_energy *= ao;

    	float3 to_light_disp = pl.position.xyz - data.worldPosition.xyz;
    	float light_distance = length(to_light_disp);
        float3 light_irradiance = pl.color.xyz * pl.color.w * attenuate_distance(pl.cutoff, light_distance) * (1.0f / 4.0f * PI);

        final_diff_energy += diff_energy * light_irradiance * n_dot_l;
        final_spec_energy += spec_energy * light_irradiance * n_dot_l;
    }

    // for each directional light calc energy
    for(uint j = 0; j < MAX_DIRECTIONAL_LIGHTS; j++){
        directional_light_t dl = DIRECTIONAL_LIGHTS[j];

        if(dl.color.w <= 0.001f){
            continue;
        }

        // Directional Light Shadow Mapping with Bias
        //if(j == 0){
        //    // transform world pos to camera view/proj
        //    float4 pos_in_camera_space = mul(float4(data.worldPosition, 1.0f), dl.view_proj);
        //    pos_in_camera_space /= pos_in_camera_space.w;

        //    // map [-1,1] to [0, 1] uv
        //    float3 uvw = (pos_in_camera_space.xyz + 1.0f) * 0.5f;
        //    uvw.y = 1.0f - uvw.y;

        //    // sample depth texture
        //    float light_depth = t_dl0_depth.Sample(s_point, uvw.xy).r;

        //    // compare depths
        //    float scene_depth = pos_in_camera_space.z;

        //    //return float4(scene_depth - light_depth, scene_depth - light_depth, scene_depth - light_depth, 1.0f);

        //    // don't contribute if depth in texture is less than depth in scene
        //    float bias = 0.001f;
        //    if(light_depth < scene_depth - bias){
        //        continue;
        //    }
        //}

        float3 light = -dl.direction.xyz;
    	float3 h = normalize(view + light);
    	float l_dot_h = saturate(dot(light, h));
    	float n_dot_h = saturate(dot(normal, h));
    	float n_dot_l = saturate(dot(normal, light));

    	float3 spec_energy_nm = calculate_spec_energy(f0_diffuse, l_dot_h, n_dot_v, n_dot_l, n_dot_h, non_linear_roughness);
    	float3 spec_energy_m = calculate_spec_energy(f0_spec, l_dot_h, n_dot_v, n_dot_l, n_dot_h, non_linear_roughness);
    	float3 spec_energy = lerp(spec_energy_nm, spec_energy_m, metalness);

    	float3 diff_energy = calculate_diffuse_energy(l_dot_h, n_dot_v, n_dot_l, non_linear_roughness);

        diff_energy *= ao;
        spec_energy *= ao;

        float3 light_irradiance = dl.color.xyz * dl.color.w * (1.0f / (PI));

        final_diff_energy += diff_energy * light_irradiance * n_dot_l;
        final_spec_energy += spec_energy * light_irradiance * n_dot_l;
    }

    // for each spot light
    for(uint k = 0; k < MAX_SPOT_LIGHTS; k++){
        spot_light_t sl = SPOT_LIGHTS[k];

        if(sl.color.w <= 0.001f){
            continue;
        }

        // Spot Light Shadow Mapping with Bias
        float shadow_factor = compute_spotlight_shadow_factor(sl, data.worldPosition.xyz);

        float3 light = normalize(sl.position.xyz - data.worldPosition.xyz);
    	float3 h = normalize(view + light);
    	float l_dot_h = saturate(dot(light, h));
    	float n_dot_h = saturate(dot(normal, h));
    	float n_dot_l = saturate(dot(normal, light));

    	float3 spec_energy_nm = calculate_spec_energy(f0_diffuse, l_dot_h, n_dot_v, n_dot_l, n_dot_h, non_linear_roughness);
    	float3 spec_energy_m = calculate_spec_energy(f0_spec, l_dot_h, n_dot_v, n_dot_l, n_dot_h, non_linear_roughness);
    	float3 spec_energy = lerp(spec_energy_nm, spec_energy_m, metalness);

    	float3 diff_energy = calculate_diffuse_energy(l_dot_h, n_dot_v, n_dot_l, non_linear_roughness);

        diff_energy *= ao;
        spec_energy *= ao;

    	float3 to_light_disp = sl.position.xyz - data.worldPosition.xyz;
    	float light_distance = length(to_light_disp);
        float3 light_irradiance = sl.color.xyz * sl.color.w * attenuate_distance(sl.cutoff, light_distance) * 
                                                              attenuate_direction(sl.direction.xyz, -light, sl.inner_angle, sl.outer_angle, sl.exp) * 
                                                              (1.0f / PI);

        final_diff_energy += diff_energy * light_irradiance * n_dot_l * shadow_factor;
        final_spec_energy += spec_energy * light_irradiance * n_dot_l * shadow_factor;
    }

	// -------------------------------------------------------------------------
	// combine terms and figure out final fragment value
    float3 final = (albedo / PI) * final_diff_energy * (1.0f - metalness);

    float3 final_spec_energy_m = final_spec_energy * f0_spec / PI;
    float3 final_spec_energy_nm = final_spec_energy * f0_diffuse / PI;
    final += lerp(final_spec_energy_nm, final_spec_energy_m, metalness);

    final += albedo * diff_ibl * ao * (1.0f - metalness);
    final += spec_ibl * ao;

    if(MIDI.BUTTON_5){
        return float4(spec_ibl, 1.0f);
    }

    if(MIDI.BUTTON_6){
        return float4(diff_ibl, 1.0f);
    }

    //final = pow(final, 1.0f / 2.2f);

	return float4(final, 1.0f);
}