#pragma once

#include "Engine/UI/ui_element.h"

class UIPanel : public UIElement
{
	public:
		Rgba m_panel_color;

	public:
		void set_color(const Rgba& panel_color);

	public:
		virtual void render() const override;
};