#include "Data/HLSL/Util/util.h"
#include "Data/HLSL/Util/cb_common.h"

// -------------------------------------------------------------
// Textures
// -------------------------------------------------------------
Texture2D <float4> t_image : register(t0);


// -------------------------------------------------------------
// Samplers
// -------------------------------------------------------------
SamplerState s_linear : register(s0);
SamplerState s_point : register(s1);


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
// Helper functions
// -------------------------------------------------------------
float offset_quality(int i)
{
    switch(i){
        case 0:
        case 1:
        case 2:
        case 3:
        case 4: return 1.0f;
        case 5: return 1.5f;
        case 6: return 2.0f;
        case 7: return 2.0f;
        case 8: return 2.0f;
        case 9: return 2.0f;
        case 10: return 4.0f;
        case 11: return 8.0f;
        default: return 8.0f;
    }
}

void step_edges(inout float2 uv1, 
                inout float2 uv2, 
                inout bool reached1, inout bool reached2, inout bool reached_both, 
                inout float luma_end_1, inout float luma_end_2, 
                in float local_average_luma, in float grad_scaled, in float2 uv_offset,
                float iteration)
{
    if(reached_both){
        return;
    }

    luma_end_1 = rgb_to_luma(t_image.Sample(s_linear, uv1).rgb);
    luma_end_1 -= local_average_luma;

    luma_end_2 = rgb_to_luma(t_image.Sample(s_linear, uv2).rgb);
    luma_end_2 -= local_average_luma;

    if(!reached1){
        reached1 = abs(luma_end_1) >= grad_scaled;
        uv1 -= uv_offset * offset_quality(iteration);
    }

    if(!reached2){
        reached2 = abs(luma_end_2) >= grad_scaled;
        uv2 += uv_offset * offset_quality(iteration);
    }

    reached_both = reached1 && reached2;
}

float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
    // calculate center luma
    float2 uv = data.texCoord;
    float3 src = t_image.Sample(s_linear, data.texCoord.xy).rgb;
    float center_luma = rgb_to_luma(src);

    // calculate the step size to move from pixel to pixel
    uint2 tex_size = get_texture_size(t_image);
    float u_step = 1.0f / (float)tex_size.x;
    float v_step = 1.0f / (float)tex_size.y;


    if(MIDI.BUTTON_0){
        return float4(src, 1.0f);
    }


    // edge detection
    const float EDGE_THRESHOLD_MIN = 0.0312f;
    const float EDGE_THRESHOLD_MAX = 0.125f;

    float3 left_src = t_image.Sample(s_linear, uv + float2(-u_step, 0.0f)).rgb;
    float3 right_src = t_image.Sample(s_linear, uv + float2(u_step, 0.0f)).rgb;
    float3 top_src = t_image.Sample(s_linear, uv + float2(0.0f, -v_step)).rgb;
    float3 bottom_src = t_image.Sample(s_linear, uv + float2(0.0f, v_step)).rgb;

    // get luma of four direct neighbors
    float left_luma = rgb_to_luma(left_src);
    float right_luma = rgb_to_luma(right_src);
    float top_luma = rgb_to_luma(top_src);
    float bottom_luma = rgb_to_luma(bottom_src);

    // get the delta of the largest and smallest
    float max_luma = max(max(left_luma, right_luma), max(top_luma, bottom_luma));
    float min_luma = min(min(left_luma, right_luma), min(top_luma, bottom_luma));
    float neighbor_range = max_luma - min_luma;

    // if the delta is sufficiently small, then we don't consider this an edge and return the original color
    if (neighbor_range < max(EDGE_THRESHOLD_MIN, max_luma * EDGE_THRESHOLD_MAX)){
        return float4(src, 1.0f);
    }



    // estimate gradient, choose edge direction
    
    float3 top_left_src = t_image.Sample(s_linear, uv + float2(-u_step, -v_step)).rgb;
    float3 top_right_src = t_image.Sample(s_linear, uv + float2(u_step, -v_step)).rgb;
    float3 bottom_left_src = t_image.Sample(s_linear, uv + float2(-u_step, v_step)).rgb;
    float3 bottom_right_src = t_image.Sample(s_linear, uv + float2(u_step, v_step)).rgb;

    // get the four corners luma
    float top_left_luma = rgb_to_luma(top_left_src);
    float top_right_luma = rgb_to_luma(top_right_src);
    float bottom_left_luma = rgb_to_luma(bottom_left_src);
    float bottom_right_luma = rgb_to_luma(bottom_right_src);

    // helper vars for later
    float bottom_top_luma = bottom_luma + top_luma;
    float left_right_luma = left_luma + right_luma;

    float left_corners_luma = bottom_left_luma + top_left_luma;
    float bottom_corners_luma = bottom_left_luma + bottom_right_luma;
    float right_corners_luma = bottom_right_luma + top_right_luma;
    float top_corners_luma = top_left_luma + top_right_luma;

    // compute local deltas for horizontal and vertical direction
    float horiz_edge = abs(-2.0f * left_luma + left_corners_luma) + 
                       abs(-2.0f * center_luma + bottom_top_luma) * 2.0f + 
                       abs(-2.0f * right_luma + right_corners_luma);

    float vert_edge = abs(-2.0f * top_luma + top_corners_luma) + 
                      abs(-2.0f * center_luma + left_right_luma) * 2.0f + 
                      abs(-2.0f * bottom_luma + bottom_corners_luma);

    // larger delta tells us what direction the edge is
    bool is_horizontal = (horiz_edge >= vert_edge);



    // edge orientation

    // select the two neighboring texels lumas in the opposite direction to the local edge
    float luma1 = is_horizontal ? top_luma : left_luma;
    float luma2 = is_horizontal ? bottom_luma : right_luma;

    // compute gradients in this direction
    float grad1 = luma1 - center_luma;
    float grad2 = luma2 - center_luma;

    // which direction has a steeper gradient?
    bool is_1_steepest = abs(grad1) >= abs(grad2);

    // gradient in the corresponding direction, normalized
    float grad_scaled = 0.25f * max(abs(grad1), abs(grad2));

    // what direction are we stepping as we walk along edges?
    float step_len = is_horizontal ? v_step : u_step;

    // average luma in the correct direction
    float local_average_luma = 0.0f;
    if (is_1_steepest){
        step_len = -step_len;
        local_average_luma = 0.5f * (luma1 + center_luma);
    }else{
        local_average_luma = 0.5f * (luma2 + center_luma);
    }

    // shift uv in the correct direction by half a pixel
    float2 current_uv = data.texCoord.xy;
    if(is_horizontal){
        current_uv.y += step_len * 0.5f;
    }else{
        current_uv.x += step_len * 0.5f;
    }


    // edge iteration

    // compute offset (for each iteration step) in the right direction
    float2 uv_offset = is_horizontal ? float2(u_step, 0.0f) : float2(0.0f, v_step);

    // compute UVs to explore on each side of the edge, orthogonally.
    float2 uv1 = current_uv - uv_offset;
    float2 uv2 = current_uv + uv_offset;

    // read lumas at each end and compute delta wrt to the local average
    float luma_end_1 = rgb_to_luma(t_image.Sample(s_linear, uv1).rgb);
    float luma_end_2 = rgb_to_luma(t_image.Sample(s_linear, uv2).rgb);
    luma_end_1 -= local_average_luma;
    luma_end_2 -= local_average_luma;

    // if the lumad deltas are larger than the local gradient, then we are at the end of that edge
    bool reached1 = abs(luma_end_1) >= grad_scaled;
    bool reached2 = abs(luma_end_2) >= grad_scaled;
    bool reached_both = reached1 && reached2;

    if(!reached1){
        uv1 -= uv_offset;
    }

    if(!reached2){
        uv2 += uv_offset;
    }

    // keep stepping until we hit the end of both directions or hit our max iteration count
    step_edges(uv1, uv2, reached1, reached2, reached_both, luma_end_1, luma_end_2, local_average_luma, grad_scaled, uv_offset, 2);
    step_edges(uv1, uv2, reached1, reached2, reached_both, luma_end_1, luma_end_2, local_average_luma, grad_scaled, uv_offset, 3);
    step_edges(uv1, uv2, reached1, reached2, reached_both, luma_end_1, luma_end_2, local_average_luma, grad_scaled, uv_offset, 4);
    step_edges(uv1, uv2, reached1, reached2, reached_both, luma_end_1, luma_end_2, local_average_luma, grad_scaled, uv_offset, 5);
    step_edges(uv1, uv2, reached1, reached2, reached_both, luma_end_1, luma_end_2, local_average_luma, grad_scaled, uv_offset, 6);
    step_edges(uv1, uv2, reached1, reached2, reached_both, luma_end_1, luma_end_2, local_average_luma, grad_scaled, uv_offset, 7);
    step_edges(uv1, uv2, reached1, reached2, reached_both, luma_end_1, luma_end_2, local_average_luma, grad_scaled, uv_offset, 8);
    step_edges(uv1, uv2, reached1, reached2, reached_both, luma_end_1, luma_end_2, local_average_luma, grad_scaled, uv_offset, 9);
    step_edges(uv1, uv2, reached1, reached2, reached_both, luma_end_1, luma_end_2, local_average_luma, grad_scaled, uv_offset, 10);
    step_edges(uv1, uv2, reached1, reached2, reached_both, luma_end_1, luma_end_2, local_average_luma, grad_scaled, uv_offset, 11);
    step_edges(uv1, uv2, reached1, reached2, reached_both, luma_end_1, luma_end_2, local_average_luma, grad_scaled, uv_offset, 12);

    // estimate offset

    // compute the distances to each extremity of the edge
    float dist1 = is_horizontal ? (uv.x - uv1.x) : (uv.y - uv1.y);
    float dist2 = is_horizontal ? (uv2.x - uv.x) : (uv2.y - uv.y);

    // in which direction is the extremity of the edge closer?
    bool is_direction_1 = dist1 < dist2;
    float dist_final = min(dist2, dist2);

    // length of the overall edge
    float edge_thickness = (dist1 + dist2);

    // UV Offset
    float pixel_offset = -dist_final / edge_thickness + 0.5f;

    // is the luma at the center smaller than the local average?
    bool is_center_luma_smaller = center_luma < local_average_luma;

    // if the luma at tcenter is smaller than at its neighbor, the delta luma at each end should be positive (same variation)
    // (in the direction of the closer side of the edge)
    bool correct_variation = ((is_direction_1 ? luma_end_1 : luma_end_2) < 0.0f) != is_center_luma_smaller;

    // if the luma variation is incorrect, do not offset.
    float final_offset = correct_variation ? pixel_offset : 0.0f;



    // subpixel antialiasing

    // full weighted average of the luma over the 3x3 neighbood
    float avg_luma = (1.0f / 12.0f) * (2.0f * (top_luma + bottom_luma + left_luma + right_luma) + 
                                               top_left_luma + top_right_luma + 
                                               bottom_left_luma + bottom_right_luma);

    // ratio of the  delta between the global average and the center luma, over the luma range in the 3x3 neighborhood
    float sub_pixel_offset_1 = clamp(abs(avg_luma - center_luma) / neighbor_range, 0.0f, 1.0f);
    float sub_pixel_offset_2 = (-2.0f * sub_pixel_offset_1 + 3.0f) * sub_pixel_offset_1 * sub_pixel_offset_1;

    // compute sub pixel offset based on this delta
    const float SUBPIXEL_QUALITY = 0.75f;
    float sub_pixel_offset_final = sub_pixel_offset_2 * sub_pixel_offset_2 * SUBPIXEL_QUALITY;

    final_offset = max(final_offset, sub_pixel_offset_final);


    // final read with computed offset and step direction
    float2 final_uv = data.texCoord.xy;
    if (is_horizontal){
        final_uv.y += final_offset * step_len;
    }else{
        final_uv.x += final_offset * step_len;
    }

    float3 final_color = t_image.Sample(s_linear, final_uv).rgb;

    return float4(final_color, 1.0f);
}