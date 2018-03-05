#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 16

// -------------------------------------------------------------
// Lighting Data Structures 
// -------------------------------------------------------------
struct directional_light_t
{
	float4 direction;
	float4 color;
};

struct point_light_t
{
	float4 position;
	float4 color;
	float2 cutoff;
	float2 _padding;
};

struct spot_light_t
{
	float4 position;
	float4 direction;
	float4 color;
	float2 cutoff;
	float inner_angle;
	float outer_angle;
	float exp;
	float3 _padding;
};

struct specular_params_t
{
	float spec_power;
	float use_texture;
	float2 _padding;
};

struct material_data_t
{
	float4 albedo;
	float4 reflectance;
	float smoothness;
	unsigned int flags;
	float2 _padding;
};

struct midi_values_t
{
	float KNOB_0;
	float KNOB_1;
	float KNOB_2;
	float KNOB_3;
	float KNOB_4;
	float KNOB_5;
	float KNOB_6;
	float KNOB_7;
	float KNOB_8;

	float SLIDER_0;
	float SLIDER_1;
	float SLIDER_2;
	float SLIDER_3;
	float SLIDER_4;
	float SLIDER_5;
	float SLIDER_6;
	float SLIDER_7;
	float SLIDER_8;

	float2 _padding;
};

cbuffer matrix_buffer_cb : register(b0)
{
	float4x4 MODEL;
	float4x4 VIEW;
	float4x4 PROJECTION;
};

cbuffer time_buffer_cb : register(b1)
{
	float GAME_TIME;
	float SYSTEM_TIME;
	float GAME_FRAME_TIME;
	float SYSTEM_FRAME_TIME;
};

cbuffer light_buffer_cb : register(b2)
{
	float4				    EYE_WORLD_POSITION;
	float4				    AMBIENT_LIGHT;
	directional_light_t	    DIRECTIONAL_LIGHTS[MAX_DIRECTIONAL_LIGHTS];
	point_light_t			POINT_LIGHTS[MAX_POINT_LIGHTS];
	spot_light_t			SPOT_LIGHTS[MAX_SPOT_LIGHTS];
	specular_params_t       SPECULAR;
};

cbuffer material_t : register(b3)
{
	material_data_t MAT_PARAMS;
};

cbuffer midi_board : register(b4)
{
	midi_values_t MIDI;
}

//----------------------------------------------------------------------
// helper functions
//----------------------------------------------------------------------

#define MAT_USE_ALBEDO_TEXTURE 0x01
#define MAT_USE_SMOOTHNESS_TEXTURE 0x02
#define MAT_USE_NORMAL_TEXTURE 0x04
#define MAT_USE_REFLECTANCE_TEXTURE 0x08
#define MAT_IS_METAL 0x16

bool mat_uses_albedo_texture(unsigned int mat_flags)
{
	return (mat_flags & MAT_USE_ALBEDO_TEXTURE) != 0;
}

bool mat_uses_smoothness_texture(unsigned int mat_flags)
{
	return (mat_flags & MAT_USE_SMOOTHNESS_TEXTURE) != 0;
}

bool mat_uses_normal_texture(unsigned int mat_flags)
{
	return (mat_flags & MAT_USE_NORMAL_TEXTURE) != 0;
}

bool mat_uses_reflectance_texture(unsigned int mat_flags)
{
	return (mat_flags & MAT_USE_REFLECTANCE_TEXTURE) != 0;
}

bool mat_is_metal(unsigned int mat_flags)
{
	return (mat_flags & MAT_IS_METAL) != 0;
}