#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Core/Rgba.hpp"

class RHITexture2D;

#define SHADOW_DISABLED 0
#define SHADOW_ENABLED 1

struct spot_light_gpu_data_t
{
	Matrix4 view_projection;
	Vector4 position;
	Vector4 direction;
	Vector4 color;
	Vector2 cutoff;
	float inner_angle;
	float outer_angle;
	float exp;
	uint is_shadow_casting;
	float _padding[2];
};

class SpotLight
{
	public:
		Vector3 m_position;
		Vector3 m_direction;

		Rgba m_color;
		float m_intensity;

		float m_near_cutoff;
		float m_far_cutoff;

		float m_inner_angle;
		float m_outer_angle;

		float m_exp;

		RHITexture2D* m_depth_rtv;
		RHITexture2D* m_depth_dsv;

	private:
		bool m_is_shadow_casting;

	public:
		SpotLight();
		SpotLight(const Vector3& position, 
				  const Vector3& direction, 
				  const Rgba& color, float intensity,
				  float near_cutoff, float far_cutoff, 
				  float inner_angle, float outer_angle, float exp,
			      bool is_shadow_casting = false);

		~SpotLight();

		void set_is_shadow_casting(bool is_shadow_casting);
		bool is_shadow_casting() const;

		Matrix4 calculate_view() const;
		Matrix4 calculate_projection() const;
		void setup_depth_write();

		spot_light_gpu_data_t to_gpu_data() const;
};