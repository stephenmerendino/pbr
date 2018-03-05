#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Core/Rgba.hpp"

class RHITexture2D;

struct directional_light_gpu_data_t
{
	Matrix4 view_projection;
	Vector4 direction;
	Vector4 color;
};

class DirectionalLight
{
	public:
		Vector3 m_direction;
		Rgba m_color;
		float m_power;

		RHITexture2D* m_depth_rtv;
		RHITexture2D* m_depth_dsv;

	private:
		bool m_is_shadow_casting;

	public:
		DirectionalLight();
		DirectionalLight(const Vector3& direction, const Rgba& color, float power, bool is_shadow_casting = false);
		~DirectionalLight();

		void set_is_shadow_casting(bool is_shadow_casting);
		bool is_shadow_casting() const;
		void setup_depth_write();
		Matrix4 calculate_view() const;
		Matrix4 calculate_projection() const;

		directional_light_gpu_data_t to_gpu_data() const;
};