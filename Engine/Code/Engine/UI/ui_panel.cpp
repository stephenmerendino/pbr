#include "Engine/UI/ui_panel.h"
#include "Engine/Engine.hpp"

void UIPanel::render() const
{
	if(m_panel_color.a > 0){
		AABB2 local_bounds = get_local_bounds();
		Matrix4 world_transform = get_world_transform();
		g_theRenderer->SetModel(world_transform);
		g_theRenderer->SetTexture(nullptr);
		g_theRenderer->DrawQuad2d(local_bounds, AABB2::ZERO_TO_ONE, m_panel_color);
	}

	if(m_debug_render_on){
		debug_render();
	}

	render_children();
}

void UIPanel::set_color(const Rgba& panel_color)
{
	m_panel_color = panel_color;
}