#pragma once

#include "Engine/UI/ui_element.h"

class RHITexture2D;

class UICanvas : public UIElement
{
	public:
		RHITexture2D* m_render_target;
		float m_target_res;

	public:
		UICanvas();
		UICanvas(RHITexture2D* render_target, float target_res);

		void set_target_res(float res);
		void set_render_target(RHITexture2D* render_target);
		void update_size();

		AABB2 calc_ortho() const;

	public:
		virtual void render() const override;
};