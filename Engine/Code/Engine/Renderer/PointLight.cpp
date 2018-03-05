#include "Engine/Renderer/PointLight.h"
#include "Engine/Renderer/CubeMap.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/ConstantBuffer.hpp"
#include "Engine/Engine.hpp"

PointLight::PointLight()
	:m_position(Vector3::ZERO)
	,m_color(Rgba::WHITE)
	,m_near_cutoff(0.1f)
	,m_far_cutoff(25.0f)
	,m_power(100.0f)
	,m_is_shadow_casting(false)
	,m_depth_rtv(nullptr)
	,m_depth_dsv(nullptr)
{
}

PointLight::PointLight(const Vector3& position, const Rgba& color, float near_cutoff, float far_cutoff, float power, bool is_shadow_casting)
	:m_position(position)
	,m_color(color)
	,m_near_cutoff(near_cutoff)
	,m_far_cutoff(far_cutoff)
	,m_power(power)
	,m_is_shadow_casting(false)
	,m_depth_rtv(nullptr)
	,m_depth_dsv(nullptr)
{
	set_is_shadow_casting(is_shadow_casting);
}

PointLight::~PointLight()
{
}

void PointLight::set_position(const Vector3& pos)
{
	m_position = pos;
}

void PointLight::set_color(const Rgba& color)
{
	m_color = color;
}

void PointLight::set_cutoff(const Vector2& cutoff)
{
	m_near_cutoff = cutoff.x;
	m_far_cutoff = cutoff.y;
}

void PointLight::set_cutoff(float near_cutoff, float far_cutoff)
{
	m_near_cutoff = near_cutoff;
	m_far_cutoff = far_cutoff;
}

void PointLight::set_near_cutoff(float near_cutoff)
{
	m_near_cutoff = near_cutoff;
}

void PointLight::set_far_cutoff(float far_cutoff)
{
	m_far_cutoff = far_cutoff;
}

void PointLight::set_power(float power)
{
	m_power = power;
}

void PointLight::set_is_shadow_casting(bool is_shadow_casting)
{
	if(m_is_shadow_casting == is_shadow_casting){
		return;
	}
	
	m_is_shadow_casting = is_shadow_casting;

	if(m_is_shadow_casting){
		m_depth_rtv = new CubeMap(256, IMAGE_FORMAT_RGBA8, false);
		m_depth_dsv = new CubeMap(256, IMAGE_FORMAT_D24S8, false);
	} else{
		SAFE_DELETE(m_depth_rtv);
		SAFE_DELETE(m_depth_dsv);
	}
}

bool PointLight::is_shadow_casting() const
{
	return m_is_shadow_casting;
}

void PointLight::setup_depth_write(uint face)
{
	m_depth_rtv->render_setup_face(face, m_position, 0);

	g_theRenderer->EnableDepth(true, true);
	g_theRenderer->SetColorTarget(m_depth_rtv, m_depth_dsv, face, 0);
	g_theRenderer->SetProjection(calculate_projection());

	g_theRenderer->m_deviceContext->ClearColorTarget(m_depth_rtv, face, Rgba::PINK);
	g_theRenderer->m_deviceContext->ClearDepthTarget(m_depth_dsv, face);
}

point_light_gpu_data_t PointLight::to_gpu_data() const
{
	point_light_gpu_data_t gpu_data;
	MemZero(&gpu_data);

	gpu_data.position = Vector4(m_position, 1.0f);

	// Set the colors as floats
	m_color.GetAsFloats(&gpu_data.color.x);
	gpu_data.color.w = m_power;

	// Set the attunuation
	gpu_data.cutoff = Vector2(m_near_cutoff, m_far_cutoff);

	return gpu_data;
}

Matrix4 PointLight::calculate_projection() const
{
	Camera c = m_depth_dsv->configure_cubemap_camera();
	c.m_nz = m_near_cutoff;
	c.m_fz = m_far_cutoff;
	return c.get_projection();
}