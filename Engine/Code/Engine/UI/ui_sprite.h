#pragma once

#include "Engine/UI/ui_element.h"
#include "Engine/UI/ui_types.h"
#include "Engine/renderer/sprite.h"

class UISprite : public UIElement
{
	public:
		Sprite* m_sprite;
		UIFillMode m_fill_mode;

	public:
		UISprite();
		~UISprite();

		void set_sprite(Sprite* sprite);
		void set_fill_mode(UIFillMode mode);

		AABB2 get_uvs() const;
		AABB2 calc_uvs_stretched() const;
		AABB2 calc_uvs_cropped() const;

	public:
		virtual void render() const override;
};