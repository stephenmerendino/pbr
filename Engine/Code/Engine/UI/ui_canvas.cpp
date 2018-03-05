#include "Engine/UI/ui_canvas.h"
#include "Engine/Engine.hpp"
#include "Engine/RHI/RHITexture2D.hpp"

UICanvas::UICanvas()
	:m_render_target(nullptr)
	,m_target_res(0.0f)
{
}

UICanvas::UICanvas(RHITexture2D* render_target, float target_res)
	:m_render_target(render_target)
	,m_target_res(target_res)
{
	update_size();
}

void UICanvas::set_target_res(float res)
{
	m_target_res = res;
	update_size();
}

void UICanvas::set_render_target(RHITexture2D* render_target)
{
	m_render_target = render_target;
	update_size();
}

void UICanvas::update_size()
{
	if(nullptr == m_render_target){
		return;
	}

	float aspect = m_render_target->get_aspect();

	float width = 0.0f;
	float height = 0.0f;
	if(aspect > 1.0f){
		height = m_target_res;
		width = aspect * m_target_res;
	} else{
		width = m_target_res;
		height = width / aspect;
	}

	m_size.units.x = width;
	m_size.units.y = height;
}

AABB2 UICanvas::calc_ortho() const
{
	if(nullptr == m_render_target){
		return AABB2(0.0f, 0.0f, 0.0f, 0.0f);
	}

	Vector2 size = get_size();

	float half_width = size.x / 2.0f;
	float half_height = size.y / 2.0f;
	AABB2 ortho_bounds(Vector2::ZERO, half_width, half_height);

	return ortho_bounds;
}

void UICanvas::render() const
{
	// set up mvp
	AABB2 ortho = calc_ortho();
	g_theRenderer->SetModel(Matrix4::IDENTITY);
	g_theRenderer->SetView(Matrix4::IDENTITY);
	g_theRenderer->SetOrthoProjection(ortho);

	// set color target
	g_theRenderer->SetColorTarget(m_render_target);

	// enable depth and blend
	g_theRenderer->EnableDepth(true, false, DEPTH_TEST_COMPARISON_LESS);
	g_theRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);

	if(m_debug_render_on){
		debug_render();
	}

	// render children
	render_children();

	//reset renderer back
	//#TODO: Do a proper reset by saving off current state of renderer?
	g_theRenderer->SetColorTarget(nullptr);
	g_theRenderer->SetModel(Matrix4::IDENTITY);
	g_theRenderer->SetView(Matrix4::IDENTITY);
	g_theRenderer->SetProjection(Matrix4::IDENTITY);
}