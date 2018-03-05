#include "Engine/UI/ui_element.h"
#include "Engine/Engine.hpp"

UIElement::UIElement()
	:m_debug_render_on(false)
	,m_debug_color(Rgba(0, 47, 255, 75))
	,m_debug_border_color(Rgba(255, 100, 0, 125))
	,m_parent(nullptr)
	,m_children(nullptr)
	,m_next_sibling(nullptr)
	,m_prev_sibling(nullptr)
	,m_positioning_mode(UI_POSITION_ABSOLUTE)
{
}

UIElement::~UIElement()
{
	destroy_children();
}

void UIElement::set_position(const Vector2& ratio, const Vector2& units)
{
	set_position(ratio.x, ratio.y, units.x, units.y);
}

void UIElement::set_position(float w_ratio, float h_ratio, float w_units, float h_units)
{
	set_position_ratio(w_ratio, h_ratio);
	set_position_units(w_units, h_units);
}

void UIElement::set_position_ratio(const Vector2& ratio)
{
	set_position_ratio(ratio.x, ratio.y);
}

void UIElement::set_position_ratio(float w_ratio, float h_ratio)
{
	m_position.ratio.x = w_ratio;
	m_position.ratio.y = h_ratio;
}

void UIElement::set_position_units(const Vector2& units)
{
	set_position_units(units.x, units.y);
}

void UIElement::set_position_units(float w_units, float h_units)
{
	m_position.units.x = w_units;
	m_position.units.y = h_units;
}

void UIElement::set_size(const Vector2& ratio, const Vector2& units)
{
	set_size(ratio.x, ratio.y, units.x, units.y);
}

void UIElement::set_size(float w_ratio, float h_ratio, float w_units, float h_units)
{
	set_size_ratio(w_ratio, h_ratio);
	set_size_units(w_units, h_units);
}

void UIElement::set_size_ratio(const Vector2& ratio)
{
	set_size_ratio(ratio.x, ratio.y);
}

void UIElement::set_size_ratio(float w_ratio, float h_ratio)
{
	m_size.ratio.x = w_ratio;
	m_size.ratio.y = h_ratio;
}

void UIElement::set_size_units(const Vector2& units)
{
	set_size_units(units.x, units.y);
}

void UIElement::set_size_units(float w_units, float h_units)
{
	m_size.units.x = w_units;
	m_size.units.y = h_units;
}

void UIElement::set_pivot(const Vector2& ratio)
{
	set_pivot(ratio.x, ratio.y);
}

void UIElement::set_pivot(float w_ratio, float h_ratio)
{
	m_pivot.ratio.x = w_ratio;
	m_pivot.ratio.y = h_ratio;
}

void UIElement::add_child_element(UIElement* child)
{
	child->m_parent = this;

	if(nullptr == m_children){
		child->m_next_sibling = child;
		child->m_prev_sibling = child;
		m_children = child;
		return;
	}

	UIElement* first_child = m_children;
	UIElement* last_child = m_children->m_prev_sibling;

	first_child->m_prev_sibling = child;
	child->m_next_sibling = first_child;

	last_child->m_next_sibling = child;
	child->m_prev_sibling = last_child;
}

void UIElement::remove_child_element(UIElement* child)
{
	// loop through children until we find child
	UIElement* first_child = m_children;
	UIElement* cur_child = first_child;
	UIElement* found_child = nullptr;
	do{
		if(child == cur_child){
			found_child = cur_child;
			break;
		}
		cur_child = cur_child->m_next_sibling;
	} while(cur_child != first_child);

	// didn't find, so early out
	if(nullptr == found_child){
		return;
	}

	// this was our only child
	if(!found_child->has_siblings()){
		m_children = nullptr;
		found_child->m_parent = nullptr;
		return;
	}

	// get the child before and after the found child
	UIElement* prev_child = found_child->m_prev_sibling;
	UIElement* next_child = found_child->m_next_sibling;

	// remove the found child from the list
	prev_child->m_next_sibling = next_child;
	next_child->m_prev_sibling = prev_child;

	// patch up the found child's sibling pointers to point to itself
	found_child->m_next_sibling = found_child;
	found_child->m_prev_sibling = found_child;

	// if children pointer was pointing at removed child, set it to next child
	if(found_child == m_children){
		m_children = next_child;
	}
}

void UIElement::remove_self()
{
	UIElement* parent = get_parent();
	if(nullptr == parent){
		return;
	}

	parent->remove_child_element(this);
}

UIElement* UIElement::get_parent()
{
	return m_parent;
}

bool UIElement::has_siblings() const
{
	if(this == m_next_sibling && this == m_prev_sibling){
		return false;
	}

	return true;
}

void UIElement::render_children() const
{
	if(nullptr != m_children){
		UIElement* first_child = m_children;
		UIElement* cur_child = first_child;
		do{
			cur_child->render();
			cur_child = cur_child->m_next_sibling;
		} while(cur_child != first_child);
	}
}

void UIElement::destroy_children()
{
	// destroy all children
	if(nullptr != m_children){
		UIElement* first_child = m_children;
		UIElement* cur_child = first_child;
		do{
			UIElement* child_to_delete = cur_child;
			cur_child = cur_child->m_next_sibling;
			SAFE_DELETE(child_to_delete);
		} while(cur_child != first_child);
	}

	m_children = nullptr;
}

Vector2 UIElement::get_size() const
{
	if(nullptr == m_parent){
		return m_size.units;
	} else{
		return m_parent->get_size() * m_size.ratio + m_size.units;
	}
}

AABB2 UIElement::get_local_bounds() const
{
	Vector2 size = get_size();
	AABB2 local_bounds(Vector2::ZERO, size);
	local_bounds.translate(-m_pivot.ratio * size);
	return local_bounds;
}

AABB2 UIElement::get_relative_bounds() const
{
	switch(m_positioning_mode){
		case UI_POSITION_ABSOLUTE:{
			return calculate_absolute_bounds();
		} break;

		default:{
			return calculate_absolute_bounds();
		} break;
	}
}

AABB2 UIElement::calculate_absolute_bounds() const
{
	if(nullptr == m_parent){
		AABB2 local_bounds = get_local_bounds();
		local_bounds.translate(m_position.units);
		return local_bounds;
	} else{
		AABB2 parent_bounds = m_parent->get_local_bounds();
		Vector2 parent_size = parent_bounds.get_size();

		Vector2 pivot_offset = parent_bounds.mins + (parent_size * m_position.ratio) + m_position.units;

		AABB2 local_bounds = get_local_bounds();
		local_bounds.translate(pivot_offset);

		return local_bounds;
	}
}

Vector2 UIElement::get_relative_position() const
{
	AABB2 relative_bounds = get_relative_bounds();
	return relative_bounds.sample(m_pivot.ratio);
}

Matrix4 UIElement::get_local_transform() const
{
	Vector2 relative_position = get_relative_position();
	Matrix4 local_translation = Matrix4::make_translation(relative_position);;
	return local_translation;
}

Matrix4 UIElement::get_world_transform() const
{
	if(nullptr == m_parent){
		return get_local_transform();
	} else{
		return get_local_transform() * m_parent->get_world_transform();
	}
}

void UIElement::set_debug_render(bool debug_render, bool propagate_to_children)
{
	m_debug_render_on = debug_render;

	if(propagate_to_children){
		if(nullptr != m_children){
			UIElement* first_child = m_children;
			UIElement* cur_child = first_child;
			do{
				cur_child->set_debug_render(debug_render, propagate_to_children);
				cur_child = cur_child->m_next_sibling;
			} while(cur_child != first_child);
		}
	}
}

void UIElement::set_debug_colors(const Rgba& debug_color, const Rgba& debug_border_color)
{
	m_debug_color = debug_color;
	m_debug_border_color = debug_border_color;
}

void UIElement::debug_render() const
{
	AABB2 local_bounds = get_local_bounds();
	Matrix4 world_transform = get_world_transform();
	g_theRenderer->SetModel(world_transform);
	g_theRenderer->DebugDrawBox2d(local_bounds, 2.0f, 2.0f, m_debug_border_color, m_debug_color);
}