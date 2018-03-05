#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Core/Rgba.hpp"

class CubeMap;
class ConstantBuffer;

struct point_light_gpu_data_t
{
	Vector4 position;
	Vector4 color;
	Vector2 cutoff;
	float _padding[2];
};

class PointLight 
{
	public:
		Vector3 m_position;
		Rgba m_color;
		float m_near_cutoff;
		float m_far_cutoff;
		float m_power;

		CubeMap* m_depth_rtv;
		CubeMap* m_depth_dsv;

	private:
		bool m_is_shadow_casting;

	public:
		PointLight();
		PointLight(const Vector3& position, const Rgba& color, float near_cutoff, float far_cutoff, float power, bool is_shadow_casting = false);
		~PointLight();

		void set_position(const Vector3& pos);
		void set_color(const Rgba& color);
		void set_cutoff(const Vector2& cutoff);
		void set_cutoff(float near_cutoff, float far_cutoff);
		void set_near_cutoff(float near_cutoff);
		void set_far_cutoff(float far_cutoff);
		void set_power(float power);

		void set_is_shadow_casting(bool is_shadow_casting);
		bool is_shadow_casting() const;

		void setup_depth_write(uint face);

		Matrix4 calculate_projection() const;

		point_light_gpu_data_t to_gpu_data() const;
};