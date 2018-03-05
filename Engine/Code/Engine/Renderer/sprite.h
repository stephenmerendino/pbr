#pragma once

#include "Engine/Math/AABB2.hpp"

class RHITexture2D;

class Sprite
{
	public:
		RHITexture2D* m_texture;
		AABB2 m_uv_region;

	public:
		Sprite();
		Sprite(const char* texture_filename, const AABB2& uv_region = AABB2::ZERO_TO_ONE);
		Sprite(RHITexture2D* texture, const AABB2& uv_region = AABB2::ZERO_TO_ONE);

		void set_texture(RHITexture2D* texture);
		void set_uv_region(const AABB2& uv_region);
};