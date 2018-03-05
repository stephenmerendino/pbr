#pragma once

#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/UI/ui_types.h"

class UIElement
{
	public:
		ui_metric_t m_position;
		ui_metric_t m_size;
		ui_metric_t m_pivot;

		UIPositioningMode m_positioning_mode;

		UIElement* m_parent;
		UIElement* m_children;
		UIElement* m_next_sibling;
		UIElement* m_prev_sibling;

		bool m_debug_render_on;
		Rgba m_debug_color;
		Rgba m_debug_border_color;

	public:
		UIElement();
		~UIElement();

		void set_position(const Vector2& ratio, const Vector2& units);
		void set_position(float w_ratio, float h_ratio, float w_units, float h_units);

		void set_position_ratio(const Vector2& ratio);
		void set_position_ratio(float w_ratio, float h_ratio);
		void set_position_units(const Vector2& units);
		void set_position_units(float w_units, float h_units);

		void set_size(const Vector2& ratio, const Vector2& units);
		void set_size(float w_ratio, float h_ratio, float w_units, float h_units);
		
		void set_size_ratio(const Vector2& ratio);
		void set_size_ratio(float w_ratio, float h_ratio);
		void set_size_units(const Vector2& units);
		void set_size_units(float w_units, float h_units);

		void set_pivot(const Vector2& ratio);
		void set_pivot(float w_ratio, float h_ratio);

		void add_child_element(UIElement* child);
		void remove_child_element(UIElement* child);
		void remove_self();
		UIElement* get_parent();
		bool has_siblings() const;
		void render_children() const;
		void destroy_children();

		Vector2 get_size() const;
		AABB2 get_local_bounds() const;
		AABB2 get_relative_bounds() const;
		AABB2 calculate_absolute_bounds() const;
		Vector2 get_relative_position() const;
		Matrix4 get_local_transform() const;
		Matrix4 get_world_transform() const;

		void set_debug_render(bool debug_render, bool propagate_to_children = false);
		void set_debug_colors(const Rgba& debug_color = Rgba::YELLOW, const Rgba& debug_border_color = Rgba::PINK);

	public:
		// eventually make these fully abstract
		virtual void render() const = 0;
		virtual void debug_render() const;

	public:
		template <typename T>
		T* add_child()
		{
			T* child = new T();
			add_child_element(child);
			return child;
		}
};