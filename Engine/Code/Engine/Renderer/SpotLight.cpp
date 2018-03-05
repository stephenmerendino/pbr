#include "Engine/Renderer/SpotLight.h"
#include "Engine/RHI/RHITexture2D.hpp"
#include "Engine/RHI/RHIInstance.hpp"
#include "Engine/Engine.hpp"

SpotLight::SpotLight()
	:m_position(0.0f, 10.0f, 0.0f)
	,m_direction(0.0f, -1.0f, 0.0f)
	,m_color(Rgba::WHITE)
	,m_near_cutoff(0.1f)
	,m_far_cutoff(25.0f)
	,m_inner_angle(30.0f)
	,m_outer_angle(70.0f)
	,m_exp(2.0f)
{
}

SpotLight::SpotLight(const Vector3& position,
					 const Vector3& direction,
					 const Rgba& color, float intensity,
					 float near_cutoff, float far_cutoff,
					 float inner_angle, float outer_angle, float exp,
                     bool is_shadow_casting)
	:m_position(position)
	,m_direction(direction)
	,m_color(color)
	,m_intensity(intensity)
	,m_near_cutoff(near_cutoff)
	,m_far_cutoff(far_cutoff)
	,m_inner_angle(inner_angle)
	,m_outer_angle(outer_angle)
	,m_exp(exp)
	,m_is_shadow_casting(false)
{
	set_is_shadow_casting(is_shadow_casting);
}

SpotLight::~SpotLight()
{
	SAFE_DELETE(m_depth_dsv);
	SAFE_DELETE(m_depth_rtv);
}

void SpotLight::set_is_shadow_casting(bool is_shadow_casting)
{
	if(m_is_shadow_casting == is_shadow_casting){
		return;
	}

	m_is_shadow_casting = is_shadow_casting;

	if(m_is_shadow_casting){
		// create depth texture
		uint shadow_res = 256;
		m_depth_dsv  = new RHITexture2D(g_theRenderer->m_device, shadow_res, shadow_res, IMAGE_FORMAT_D24S8);
		m_depth_rtv = new RHITexture2D(g_theRenderer->m_device, shadow_res, shadow_res, IMAGE_FORMAT_RGBA8);
	} else{
		// destroy depth texture
		SAFE_DELETE(m_depth_dsv );
		SAFE_DELETE(m_depth_rtv);
	}
}

bool SpotLight::is_shadow_casting() const
{
	return m_is_shadow_casting;
}

Matrix4 SpotLight::calculate_view() const
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

	Matrix4 inv_trans = Matrix4::make_translation(-m_position);
	Matrix4 view = inv_trans * rot.transposed();

	return view;
}

Matrix4 SpotLight::calculate_projection() const
{
	Matrix4 perspective;
	float angle = max(m_inner_angle, m_outer_angle) * 2.0f;
	perspective = RHIInstance::GetInstance().CreatePerspectiveProjection(m_near_cutoff, m_far_cutoff, angle, 1);
	return perspective;
}

void SpotLight::setup_depth_write()
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

spot_light_gpu_data_t SpotLight::to_gpu_data() const
{
	spot_light_gpu_data_t gpu_data;
	MemZero(&gpu_data);

	gpu_data.position = Vector4(m_position, 1.0f);
	gpu_data.direction = Vector4(m_direction, 1.0f);

	Vector4 color_norm = m_color.GetAsVector4Normalized();
	gpu_data.color = Vector4(color_norm.xyz, m_intensity);

	gpu_data.cutoff = Vector2(m_near_cutoff, m_far_cutoff);
	gpu_data.inner_angle = m_inner_angle;
	gpu_data.outer_angle = m_outer_angle;
	gpu_data.exp = m_exp;

	Matrix4 view = calculate_view();
	Matrix4 proj = calculate_projection();
	gpu_data.view_projection = view * proj;

	gpu_data.is_shadow_casting = m_is_shadow_casting ? SHADOW_ENABLED : SHADOW_DISABLED;

	return gpu_data;
}