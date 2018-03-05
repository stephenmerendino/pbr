#include "Engine/UI/ui_sprite.h"
#include "Engine/Engine.hpp"
#include "Engine/RHI/RHITexture2D.hpp"

UISprite::UISprite()
	:m_sprite(nullptr)
	, m_fill_mode(UI_FILL_STRETCH)
{
}

UISprite::~UISprite()
{
	SAFE_DELETE(m_sprite);
}

void UISprite::set_sprite(Sprite* sprite)
{
	m_sprite = sprite;
}

void UISprite::set_fill_mode(UIFillMode mode)
{
	m_fill_mode = mode;
}

AABB2 UISprite::get_uvs() const
{
	switch(m_fill_mode){
	case UI_FILL_STRETCH:{
		return calc_uvs_stretched();
	} break;

	case UI_FILL_CROP:{
		return calc_uvs_cropped();
	} break;

	default:{
		return calc_uvs_stretched();
	} break;
	}
}

AABB2 UISprite::calc_uvs_stretched() const
{
	return m_sprite->m_uv_region;
}

AABB2 UISprite::calc_uvs_cropped() const
{
	if(nullptr == m_sprite){
		return AABB2::ZERO_TO_ONE;
	}

	AABB2 local_bounds = get_local_bounds();
	float bounds_ar = local_bounds.get_aspect();
	float texture_ar = m_sprite->m_texture->get_aspect();

	if(AreMostlyEqual(bounds_ar, texture_ar)){
		return m_sprite->m_uv_region;
	}

	AABB2 uv_region = m_sprite->m_uv_region;
	float uv_width = uv_region.CalcWidth();
	float uv_height = uv_region.CalcHeight();

	if(bounds_ar > texture_ar){
		float ar_ratio = texture_ar / bounds_ar;

		AABB2 uvs;
		uvs.mins.x = uv_region.mins.x;
		uvs.maxs.x = uv_region.maxs.x;

		uvs.mins.y = uv_region.mins.y + (uv_height * (1.0f - ar_ratio));
		uvs.maxs.y = uv_region.maxs.y;

		float height_correction = -m_pivot.ratio.y * (1.0f - ar_ratio);
		uvs.translate(Vector2(0.0f, height_correction));

		return uvs;
	} else{
		float ar_ratio = bounds_ar / texture_ar;

		AABB2 uvs;

		uvs.mins.x = uv_region.mins.x;
		uvs.maxs.x = uv_region.maxs.x - (uv_width * (1.0f - ar_ratio));

		uvs.mins.y = uv_region.mins.y;
		uvs.maxs.y = uv_region.maxs.y;

		float width_correction = m_pivot.ratio.x * (1.0f - ar_ratio);
		uvs.translate(Vector2(width_correction, 0.0f));

		return uvs;
	}
}

void UISprite::render() const
{
	AABB2 local_bounds = get_local_bounds();
	Matrix4 world_transform = get_world_transform();
	g_theRenderer->SetModel(world_transform);

	g_theRenderer->SetTexture(m_sprite->m_texture);

	AABB2 uvs = get_uvs();
	g_theRenderer->DrawQuad2d(local_bounds, uvs);

	if(m_debug_render_on){
		debug_render();
	}

	render_children();
}