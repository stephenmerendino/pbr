#include "Engine/Renderer/DirectionalLight.h"
#include "Engine/Engine.hpp"
#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/RHI/RHIInstance.hpp"

DirectionalLight::DirectionalLight()
	:m_direction(-Vector3::Y_AXIS)
	,m_color(Rgba::WHITE)
	,m_power(100.0f)
{
}

DirectionalLight::DirectionalLight(const Vector3& direction, const Rgba& color, float power, bool is_shadow_casting)
	:m_direction(direction)
	,m_color(color)
	,m_power(power)
	,m_is_shadow_casting(is_shadow_casting)
{
}

DirectionalLight::~DirectionalLight()
{
	SAFE_DELETE(m_depth_rtv);
	SAFE_DELETE(m_depth_dsv );
}

void DirectionalLight::set_is_shadow_casting(bool is_shadow_casting)
{
	if(m_is_shadow_casting == is_shadow_casting){
		return;
	}

	m_is_shadow_casting = is_shadow_casting;

	if(m_is_shadow_casting){
		// create depth texture
		m_depth_dsv  = new RHITexture2D(g_theRenderer->m_device, 256, 256, IMAGE_FORMAT_D24S8);
		m_depth_rtv = new RHITexture2D(g_theRenderer->m_device, 256, 256, IMAGE_FORMAT_RGBA8);
	} else{
		// destroy depth texture
		SAFE_DELETE(m_depth_dsv );
		SAFE_DELETE(m_depth_rtv);
	}
}

bool DirectionalLight::is_shadow_casting() const
{
	return m_is_shadow_casting;
}

Matrix4 DirectionalLight::calculate_view() const
{
	Matrix4 rot;

	Vector3 dir_norm = m_direction.Normalized();
	Vector3 up;

	if(abs(dir_norm.y) < 0.999f){
		up = Vector3::Y_AXIS;
	} else{
		up = Vector3::Z_AXIS;
	}

	Vector3 k = dir_norm;
	Vector3 i = CrossProduct(up, k);
	Vector3 j = CrossProduct(k, i);
	rot = Matrix4(i, j, k);

	return rot.transposed();
}

Matrix4 DirectionalLight::calculate_projection() const
{
	float radius = 15.0f;
	Matrix4 ortho;
	ortho = RHIInstance::GetInstance().CreateOrthoProjection(-radius, radius, -radius, radius, -radius, radius);
	return ortho;
}

void DirectionalLight::setup_depth_write()
{
	// setup view & projection
	Matrix4 view = calculate_view();
	g_theRenderer->SetView(view);

	Matrix4 ortho = calculate_projection();
	g_theRenderer->SetProjection(ortho);

	// bind dsv as depth stencil
	g_theRenderer->SetColorTarget(m_depth_rtv, m_depth_dsv);

	// clear dsv
	g_theRenderer->ClearColor(Rgba::PINK);
	g_theRenderer->ClearDepth();

	g_theRenderer->SetViewport(0, 0, m_depth_rtv->GetWidth(), m_depth_rtv->GetHeight());
}

directional_light_gpu_data_t DirectionalLight::to_gpu_data() const
{
	directional_light_gpu_data_t gpu_data;
	MemZero(&gpu_data);

	// dir
	gpu_data.direction = Vector4(m_direction, 1.0f);

	// color/power
	Vector4 color_norm = m_color.GetAsVector4Normalized();
	gpu_data.color = Vector4(color_norm.xyz, m_power);

	// view/proj
	Matrix4 view = calculate_view();
	Matrix4 ortho = calculate_projection();

	Matrix4 view_proj = view * ortho;
	gpu_data.view_projection = view_proj;

	return gpu_data;
}