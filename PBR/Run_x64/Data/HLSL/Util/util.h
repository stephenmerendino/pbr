#ifndef UTIL_H
#define UTIL_H

#define PI 3.1415926535897932384626433832795f

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

#endif