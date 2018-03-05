#ifndef UTIL_H
#define UTIL_H

#define PI 3.1415926535897932384626433832795f
#define TWO_PI 6.283185307179586476925286766559f
#define INV_PI 0.31830988618379067153776752674503f
#define INV_TWO_PI 0.15915494309189533576888376337251f

float3 texture_normal_to_surface_normal(float3 texture_normal)
{
    float3 tbn_normal = (texture_normal * float3(2.0, 2.0, 1.0)) - float3(1.0, 1.0, 0.0);
    tbn_normal = normalize(tbn_normal);
    return tbn_normal;
}

float4 surface_normal_to_color(float3 surface_normal)
{
    return float4(surface_normal + float3(1.0, 1.0, 1.0) * 0.5, 1.0);
}

float deg_to_rad(float deg)
{
	return (deg * (PI / 180.0f));
}

float rad_to_deg(float rad)
{
	return (rad * (180.0f / PI));
}

float range_map(float val, float in_min, float in_max, float out_min, float out_max){
	float in_range = in_max - in_min;
	float in_normalized = (val - in_min) / in_range;

	float out_range = out_max - out_min;

	return (in_normalized * out_range) + out_min;
}

uint2 get_texture_size(Texture2D<float4> tex)
{
	uint window_w = 0;
	uint window_h = 0;
	uint num_levels = 0;

	tex.GetDimensions(0, window_w, window_h, num_levels);

	return uint2(window_w, window_h);
}

float rgb_to_luma(float3 rgb)
{
    return sqrt(dot(rgb, float3(0.299, 0.587, 0.114)));
}


#endif